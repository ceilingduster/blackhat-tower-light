FROM python:3.10
COPY main.py /
COPY run.sh /
COPY requirements.txt /
COPY mqttlib.py /
RUN chmod +x /run.sh
RUN pip3 install -r /requirements.txt
CMD [ "/run.sh" ]
