
# Use the local tip container image
FROM localhost/centostip:1

RUN python3 -m pip install pyyaml

# Create a new user to avoid root
# Note: current tip image which results from pipeline
# does not have useradd.
#RUN useradd -ms /bin/bash appuser

# Add script and set permissions
WORKDIR /app
ADD run_tip.py /app
#RUN chown -R appuser:appuser /app \
#    && chmod a+x /app/run_tip.py

# Switch to non-root user.
#USER user

ENTRYPOINT ["python3", "/app/run_tip.py"]