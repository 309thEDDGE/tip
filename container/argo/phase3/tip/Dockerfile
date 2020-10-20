
# Use the local tip container image
FROM registry.il2.dsop.io/skicamp/project-opal/tip:93a3c442

RUN pip install pyyaml

# Create a new user to avoid root
# Note: current tip image which results from pipeline
# does not have useradd.
#RUN useradd -ms /bin/bash user

# Add script and set permissions
ADD run_tip.py /app
#RUN chown -R user:user /app \
#    && chmod a+x /app/run_tip.py

# Switch to non-root user.
#USER user

ENTRYPOINT ["python3", "/app/run_tip.py"]