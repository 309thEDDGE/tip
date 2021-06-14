import subprocess
import json
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("environment")
parser.add_argument("output_file", nargs="?", default="mirror_config.yaml", type=str)
args = parser.parse_args()

text = subprocess.run(["conda", "env", "export", "-n", args.environment, "--json"], capture_output=True).stdout.decode('utf-8')
env_dict = json.loads(text)

whitelist = """blacklist:
    - name: \"*\"
whitelist:\n"""

for entry in env_dict['dependencies']:
    parts = entry.split('=')
    whitelist += f"""    - name: \"{parts[0]}\"
      version: \"{parts[1]}\"
      build: \"{parts[2]}\"\n"""
    
print(whitelist)

with open("mirror_config.yaml", 'w') as f:
    f.write(whitelist)
print("\n\n----------------------------\n\n")
print(f'Above whitelist config written to {args.output_file}')
