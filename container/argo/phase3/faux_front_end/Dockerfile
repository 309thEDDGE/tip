
# Get a light weight python image.
# docker pull python
FROM python:latest

# Create a new user to avoid root
RUN useradd -ms /bin/bash user

# Create app dir.
RUN mkdir /app \
	&& chown -R user:user /app 

# Install pyyaml
RUN pip install pyyaml

# Add script
ADD yaml_from_input_args.py /app
RUN chmod a+x /app/yaml_from_input_args.py

# Switch to non-root user.
USER user

# Entrypoint is the python script with input args retrieved
# from container instantiation:
#
# arg 1 - ch10 data dir mount point
#
ENTRYPOINT ["python", "/app/yaml_from_input_args.py"]

