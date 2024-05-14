FROM azul/zulu-openjdk:11-jre-headless-latest

RUN apt-get update && \
    apt-get install netcat git -y && \
    apt-get upgrade -y && \
    apt-get autoremove && \
    apt-get autoclean && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /git
RUN git clone https://github.com/alliander-opensource/ReLevENT.git

WORKDIR /git/ReLevENT/hedera-61850-gateway/
RUN /bin/bash gradlew  shadowJar && mkdir -p /srv/hedera-61850-gateway/ && cp demo-build/gateway-app-0.1-SNAPSHOT-all.jar /srv/hedera-61850-gateway/

WORKDIR /srv/hedera-61850-gateway
ENTRYPOINT ["java", "-jar", "/srv/hedera-61850-gateway/gateway-app-0.1-SNAPSHOT-all.jar"]
