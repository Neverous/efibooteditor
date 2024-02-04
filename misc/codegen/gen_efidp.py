# Generate sources from EFI device-paths.yml and Jinja2 templates

import logging
import sys

import jinja2
import yaml

log = logging.getLogger("gen_efidp")


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    env = jinja2.Environment(trim_blocks=True, lstrip_blocks=True)
    device_paths = yaml.safe_load(open(sys.argv[1]).read())
    for category in device_paths.values():
        for node in category["nodes"]:
            node["fields_by_slug"] = {field["slug"]: field for field in node["fields"]}

    for file in sys.argv[2:]:
        log.info("Processing %s file", file)
        template = env.from_string(open(file).read())
        with open(file.removesuffix(".j2"), "w") as fd:
            fd.write(template.render({"device_paths": device_paths}))
