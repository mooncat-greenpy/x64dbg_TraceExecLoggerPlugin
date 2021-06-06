from jsonschema import validate, RefResolver
import json
import os
import sys


def validate_file(path):
    print(path)
    f = open(path)
    json_data = json.load(f)
    f.close()

    f = open("schema.json")
    schema = json.load(f)
    f.close()

    schema_path = "file:///{0}/".format((os.getcwd()).replace("\\", "/"))
    ref = RefResolver(schema_path, schema)
    validate(instance=json_data, schema=schema, resolver=ref)
    print("OK")


def main():
    if len(sys.argv) < 2:
        return
    for i in os.listdir(sys.argv[1]):
        validate_file(os.path.join(sys.argv[1], i))


if __name__ == "__main__":
    main()
