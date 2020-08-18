
FROM registry.redhat.io/rhel8/python-36:latest

ENV DNF="dnf install -y"

#
# Update package manager and repos.
# There was an error running update. It wasn't necessary to
# get the requirements.txt.
#
#RUN dnf update -y && dnf clean all && dnf upgrade -y

WORKDIR /app

COPY ./requirements.txt /app/requirements.txt

RUN pip install -r requirements.txt

COPY . /app

#
# Make the app dir writeable by the default user
#
USER root
RUN chown -R default /app
USER default

#
# Remove the database that may exist within the imported files.
#
RUN rm tws.db

EXPOSE 8000

ENTRYPOINT [ "gunicorn","-b",":8000","--access-logfile","-","--error-logfile","-","-w","1","tws:app" ]