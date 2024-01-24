FROM azul/zulu-openjdk:11-jre-headless-latest

RUN apt-get update && \
    apt-get install netcat -y && \
    apt-get upgrade -y && \
    apt-get autoremove && \
    apt-get autoclean && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
