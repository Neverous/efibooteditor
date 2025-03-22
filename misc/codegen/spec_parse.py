# Parse Device Path nodes from UEFI specification HTML site into yaml

import dataclasses
import logging
import re
import sys
import urllib.request
from typing import Any

import bs4
import yaml


@dataclasses.dataclass
class DevicePathNodeField:
    name: str
    slug: str = dataclasses.field(init=False)
    description: str
    offset: int | str
    size: int | str
    # For CodeGen
    type: str = "int"

    def __post_init__(self) -> None:
        self.slug = slugify(self.name)


@dataclasses.dataclass
class DevicePathNode:
    name: str
    slug: str = dataclasses.field(init=False)
    description: str
    type: int
    subtype: int
    fields: list[DevicePathNodeField] = dataclasses.field(default_factory=list)

    def __post_init__(self) -> None:
        self.slug = slugify(self.name)


@dataclasses.dataclass
class DevicePathCategory:
    name: str
    slug: str = dataclasses.field(init=False)
    description: str
    type: int
    nodes: list[DevicePathNode] = dataclasses.field(default_factory=list)

    def __post_init__(self) -> None:
        self.slug = slugify(self.name)


@dataclasses.dataclass
class DevicePaths:
    hardware: DevicePathCategory
    acpi: DevicePathCategory
    messaging: DevicePathCategory
    media: DevicePathCategory
    bios: DevicePathCategory
    end: DevicePathCategory

    def __init__(self) -> None:
        super().__init__()
        self.hardware = DevicePathCategory("Hardware", "", 0x01)
        self.acpi = DevicePathCategory("ACPI", "", 0x02)
        self.messaging = DevicePathCategory("Messaging", "", 0x03)
        self.media = DevicePathCategory("Media", "", 0x04)
        self.bios = DevicePathCategory("BIOS", "", 0x05)
        self.end = DevicePathCategory(
            "End",
            "",
            0x7F,
            [
                DevicePathNode("End This Instance", "", 0x7F, 0x01),
                DevicePathNode("End Entire", "", 0x7F, 0xFF),
            ],
        )

    def verify(self) -> bool:
        success = True
        for field in dataclasses.fields(self)[:-1]:
            category = getattr(self, field.name)
            category.nodes.sort(key=lambda node: node.subtype)
            id_ = 1
            for node in category.nodes:
                while node.subtype > id_:
                    log.warning("%s is missing subtype %d!", category.name, id_)
                    success = False
                    id_ += 1

                if node.subtype < id_:
                    log.warning("%s has duplicated subtype %d!", category.name, node.subtype)
                    continue

                id_ += 1

        return success


re_device_path = re.compile(r"^.*device-path.*$", re.IGNORECASE)
re_type = re.compile(r"^type -?0?x?\d+ (-? )?(.*)$", re.IGNORECASE)
re_subtype = re.compile(r"^(sub.type)? ?(\d+)[ -].*$", re.IGNORECASE)
re_node_name = re.compile(r" device paths?.*$", re.IGNORECASE)
re_slug = re.compile(r"\W+")
re_description = re.compile(r"\n\W+")

SPEC_URL = "https://uefi.org/specs/UEFI/2.11/10_Protocols_Device_Path_Protocol.html"

log = logging.getLogger("spec_parse")


class ListIndent(yaml.SafeDumper):
    def __init__(self, *args: Any, **kwargs: Any):
        super().__init__(*args, **kwargs)

        def str_presenter(dumper: ListIndent, data: str) -> yaml.Node:
            if "\n" in data:
                return dumper.represent_scalar("tag:yaml.org,2002:str", data, style="|")
            return dumper.represent_scalar("tag:yaml.org,2002:str", data)

        self.add_representer(str, str_presenter)

    def increase_indent(self, flow: bool = False, indentless: bool = False) -> None:
        return super().increase_indent(flow, False)


def slugify(text: str) -> str:
    return re_slug.sub("_", text).lower()


def maybe_int(text: str) -> int | str:
    try:
        return int(text)
    except ValueError:
        return text


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    if len(sys.argv) > 1:
        SPEC_URL = sys.argv[1]

    log.info("Parsing Device Path nodes from %s into yaml", SPEC_URL)
    req = urllib.request.Request(SPEC_URL, headers = {
        'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0',
        })
    soup = bs4.BeautifulSoup(
        urllib.request.urlopen(req).read().decode("utf-8").encode("ascii", "ignore"), features="html.parser"
    )

    device_paths = DevicePaths()
    nodes_tag = soup.find("div", id="device-path-nodes")
    if not nodes_tag:
        nodes_tag = soup.find("section", id="device-path-nodes")

    assert isinstance(nodes_tag, bs4.Tag)
    for table in nodes_tag.find_all("table")[2:]:
        caption = table.caption("span")[1].text.strip()
        if (header := slugify(table.tr.text.strip())) != "mnemonic_byte_offset_byte_length_description":
            log.warning('Skipping "%s" table: invalid header "%s"', caption, header)
            continue

        fields = table("tr")
        if not (match := re_type.match(fields[1]("td")[3].text.strip())):
            log.warning('Skipping "%s" table: type didn\'t match', caption)
            continue

        category = getattr(device_paths, match.group(2).split(" ", 1)[0].lower())
        type = category.type
        if not (match := re_subtype.match(fields[2]("td")[3].text.strip())):
            log.warning('Skipping "%s" table: subtype didn\'t match', caption)
            continue

        node_name = re_node_name.sub("", caption)
        subtype = int(match.group(2))
        node = DevicePathNode(node_name, "", type, subtype)

        for field in fields[4:]:
            field_name = (field("td")[0].p.contents or " ")[0].strip().strip("_") or "Data"
            node_field = DevicePathNodeField(
                field_name,
                re_description.sub("\n", field("td")[3].text.strip()),
                maybe_int(field("td")[1].text.strip().lower()),
                maybe_int(field("td")[2].text.strip().lower()),
            )

            if isinstance(node_field.size, str):
                node_field.type = "raw_data"

            node.fields.append(node_field)

        category.nodes.append(node)
        log.info('Parsed "%s" table', caption)

    # Verify
    device_paths.verify()

    print(yaml.dump(dataclasses.asdict(device_paths), Dumper=ListIndent, sort_keys=False, indent=2))
