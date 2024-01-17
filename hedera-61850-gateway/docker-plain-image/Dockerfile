FROM azul/zulu-openjdk:11-jre-headless-latest

COPY java-build/gateway-app-0.1-SNAPSHOT-all.jar /srv/hedera-61850-gateway/
WORKDIR /srv/hedera-61850-gateway/

RUN apt-get update && \
    apt-get install netcat -y && \
    apt-get upgrade -y && \
    apt-get autoremove && \
    apt-get autoclean && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["java", "-jar", "/srv/hedera-61850-gateway/gateway-app-0.1-SNAPSHOT-all.jar"]