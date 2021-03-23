import os, sys
import shutil, os

output_path = ""
if len(sys.argv) != 2:
	print("\nOutput path not provided as command line argument")
	print("Setting output path to current directory\n")
	output_path = "."
else:
	output_path = sys.argv[1]

if not os.path.isdir(output_path):
	print("ERROR- output path does not exist: " + output_path)
	sys.exit(0)

if os.path.isdir(os.path.join(output_path,'deliverable')):
	print("ERROR- path already exists: " + output_path + '\\deliverable')
	sys.exit(0)

# Get the tip path so you can run the script from anywhere
tip_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..\\')

# Check to make sure necessary directories exist
if not os.path.isdir(os.path.join(tip_path,'bin')):
	print("ERROR- non existant bin dir: " + tip_path + '\\bin' + "\nmust build first \n")
	sys.exit(0)

if not os.path.isdir(os.path.join(tip_path,'conf')):
	print("ERROR- non existant conf dir: " + tip_path + '\\conf')
	sys.exit(0)

if not os.path.isdir(os.path.join(tip_path,'tip_scripts')):
	print("ERROR- non existant scripts path: " + tip_path + '\\tip_scripts')
	sys.exit(0)


os.mkdir(os.path.join(output_path,'deliverable'))
os.mkdir(os.path.join(output_path,'deliverable','bin'))
os.mkdir(os.path.join(output_path,'deliverable','tip_scripts'))

# Copy all .exe files from bin
for root, dirs, files in os.walk(os.path.join(tip_path, 'bin')):
    for file in files:
        if file.endswith(".exe"):
             q = os.path.join(root, file)
             print(q + " ------> copy done")              
             shutil.copy2(q , os.path.join(output_path, 'deliverable','bin'))

# Copy default_conf
shutil.copytree(os.path.join(tip_path, 'conf', 'default_conf'), os.path.join(output_path, 'deliverable','conf', 'default_conf'))
print(os.path.join(tip_path, 'conf', 'default_conf') + " ------> copy done") 

# Copy yaml_schemas
shutil.copytree(os.path.join(tip_path, 'conf', 'yaml_schemas'), os.path.join(output_path, 'deliverable','conf', 'yaml_schemas'))
print(os.path.join(tip_path, 'conf', 'yaml_schemas') + " ------> copy done") 

# Copy main run script
shutil.copy2(os.path.join(tip_path, 'parse_and_translate.py'), os.path.join(output_path, 'deliverable'))
print(os.path.join(tip_path, 'parse_and_translate.py')+ " ------> copy done") 

# Copy main run script dependencies
shutil.copy2(os.path.join(tip_path, 'tip_scripts', 'run_cl_process.py'), os.path.join(output_path, 'deliverable', 'tip_scripts'))
print(os.path.join(tip_path, 'tip_scripts', 'run_cl_process.py') + " ------> copy done") 

shutil.copy2(os.path.join(tip_path, 'tip_scripts', 'exec.py'), os.path.join(output_path, 'deliverable', 'tip_scripts'))
print(os.path.join(tip_path, 'tip_scripts', 'exec.py') + " ------> copy done") 

# Copy the README
shutil.copy2(os.path.join(tip_path, 'tip_scripts', 'DELIVERABLE_README.txt'), os.path.join(output_path, 'deliverable', 'README.txt'))
print(os.path.join(tip_path, 'tip_scripts', 'DELIVERABLE_README.txt') + " ------> copy done") 

print("\n\nCopied deliverable folder to:  " + output_path + "\\deliverable\n")