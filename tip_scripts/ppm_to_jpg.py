
import os
from PIL import Image

#print(os.getcwd())

currdir = os.getcwd()

for ppm_file in [x for x in os.listdir(currdir) if x.find('.ppm') > 0]:
	im = Image.open(os.path.join(currdir, ppm_file))
	im.save(os.path.join(currdir, ppm_file[:-4] + '.jpg'))