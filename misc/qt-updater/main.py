# Fetch latest qt versions and update CI config

import logging
import re
import sys
from dataclasses import dataclass
from datetime import date
from typing import Any

import requests
import ruamel.yaml


@dataclass
class Version:
    major: int
    minor: int
    patch: int | None = None

    def nopatch(self) -> "Version":
        return Version(self.major, self.minor)

    def __hash__(self) -> int:
        return hash((self.major, self.minor, self.patch))

    def __str__(self) -> str:
        return f"{self.major}.{self.minor}" + (f".{self.patch}" if self.patch else "")

    def __repr__(self) -> str:
        return str(self)

    def __lt__(self, obj: "Version") -> bool:
        return (self.major, self.minor, self.patch or 0) < (
            obj.major,
            obj.minor,
            obj.patch or 0,
        )


@dataclass
class QtVersion:
    version: Version
    eol: date
    source: str = ""

    def __hash__(self) -> int:
        return hash(self.version)

    def comment(self) -> str:
        return "Supported" + (" in " + self.source if self.source else "") + f" until {self.eol}"

    def __str__(self) -> str:
        return f"{self.version} ({self.comment()})"

    def __repr__(self) -> str:
        return str(self)

    def __lt__(self, obj: "QtVersion") -> bool:
        return self.version < obj.version


log = logging.getLogger("qt-update")
api = requests.session()


def fetch_eoldate_info(name: str) -> Any:
    log.debug("Fetching %s info from endoflife.date", name)
    return api.get(f"https://endoflife.date/api/{name}.json").json()


def fetch_ubuntu_package_versions(series_slug: str, name: str) -> set[Version]:
    log.debug("Fetching %s packages info from Ubuntu %s", name, series_slug)
    packages = api.get(
        "https://api.launchpad.net/1.0/ubuntu/+archive/primary",
        params={
            "ws.op": "getPublishedBinaries",
            "binary_name": name,
            "exact_match": "true",
            "distro_arch_series": f"https://api.launchpad.net/1.0/ubuntu/{series_slug}/amd64",
            "pocket": "Release",
            "status": "Published",
        },
    ).json()["entries"]
    return {Version(*map(int, package["binary_package_version"].split(".")[:2])) for package in packages}


def fetch_installable_qt_versions() -> set[Version]:
    log.debug("Fetching installable Qt versions")
    listing = api.get("https://download.qt.io/online/qtsdkrepository/linux_x64/desktop/").text
    return {
        Version(*map(int, filter(None, version))) for version in re.findall(r'href="qt(\d)_\1(\d{0,2})(\d+)/"', listing)
    }


def get_supported_qt_versions(supported_date: date) -> tuple[dict[Version, QtVersion], set[Version]]:
    log.debug("Getting supported Qt versions")
    versions = {}
    lts_releases = set()

    for cycle in fetch_eoldate_info("qt"):
        eol = date.fromisoformat(cycle["eol"])
        version = Version(*map(int, cycle["cycle"].split(".")))
        if cycle["lts"]:
            log.debug("Adding %s to LTS versions", version)
            lts_releases.add(version)

        if eol < supported_date:
            continue

        log.info("Adding %s to supported versions (until %s)", version, eol)
        versions[version] = QtVersion(version, eol, "")

    return versions, lts_releases


def find_used_qt_versions(supported_date: date) -> set[QtVersion]:
    log.debug("Finding used Qt versions")

    versions, lts_releases = get_supported_qt_versions(supported_date)

    # Check for latest versions in supported Ubuntu LTS
    for cycle in fetch_eoldate_info("ubuntu"):
        eol = date.fromisoformat(cycle["eol"])
        if eol < supported_date:
            continue

        series = cycle["codename"]
        log.debug("Checking Qt packages in Ubuntu %s (supported until %s)", series, eol)

        series_slug = cycle["codename"].split()[0].lower()
        for package in ("qtbase5-dev", "qt6-base-dev"):
            for version in fetch_ubuntu_package_versions(series_slug, package):
                if version not in lts_releases:
                    log.debug("Skipping non-LTS version: %s (from Ubuntu %s)", version, series)
                    continue

                log.info("Adding %s to supported versions (from Ubuntu %s)", version, series)
                if version not in versions or versions[version].eol < eol:
                    versions[version] = QtVersion(version, eol, f"Ubuntu {series}")

    return set(versions.values())


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)

    log.info("Compiling target Qt versions")
    now = date.today()
    latest = {}
    for version in fetch_installable_qt_versions():
        minor = version.nopatch()
        if latest.get(minor, Version(0, 0)) < version:
            latest[minor] = version

    target_versions = sorted(find_used_qt_versions(now))
    for qt_version in target_versions:
        qt_version.version = latest[qt_version.version]

    log.info("Compiled target Qt versions: %s", target_versions)
    log.info("Processing input workflow files...")

    yaml = ruamel.yaml.YAML()
    yaml.width = 1024
    yaml.indent(mapping=2, sequence=4, offset=2)
    for file in sys.argv[1:]:
        log.info("Processing %s file", file)
        with open(file, "rb") as fr:
            workflow = yaml.load(fr)

        for key, job in workflow.get("jobs", {}).items():
            if not job.get("strategy", {}).get("matrix", {}).get("qt-version"):
                continue

            job_versions = job["strategy"]["matrix"]["qt-version"]
            # Only set latest version for winget-update job
            if key == "winget-update":
                target = target_versions[-1]
                job_versions[0] = str(target.version)
                job_versions.yaml_add_eol_comment(target.comment(), 0)
                continue

            job_versions.clear()
            for t, target in enumerate(target_versions):
                job_versions.append(str(target.version))
                job_versions.yaml_add_eol_comment(target.comment(), t)

        with open(file, "wb") as fw:
            yaml.dump(workflow, fw)
