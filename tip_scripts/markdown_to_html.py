# Requires markdown2.py to be in PYTHONPATH
# markdown2.py is approved for SWEG

import markdown2
from pathlib import Path
import sys, os
import codecs

if __name__ == '__main__':

	if len(sys.argv) < 2:
		print('Need input *.md path')
		sys.exit(0)
		
	input_md_file = sys.argv[1]
	if not os.path.exists(input_md_file):
		print('File {:s} does not exist'.format(input_md_file))
		sys.exit(0)
	#print(input_md_file)
	
	input_path = Path(input_md_file)
	output_html_file = input_path.with_suffix('.html')
	#print(output_html_file.resolve())
	
	#html = markdown2.markdown_path(input_md_file)
	
	fp = codecs.open(input_md_file, 'r', "utf-8")
	text = fp.read()
	fp.close()
	extras = {'header-ids': None}
	html = markdown2.Markdown(html4tags=True, extras=extras).convert(text)
	with open(output_html_file.resolve(), 'w') as f:
		f.write(html)
		
	sys.exit(0)
