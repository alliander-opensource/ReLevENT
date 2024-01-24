FROM relevent-61850-hedera-gateway-plain:0.1

WORKDIR /srv/hedera-61850-gateway/
COPY java-build/gateway-app-0.1-SNAPSHOT-all.jar /srv/hedera-61850-gateway/
COPY hedera-interface.ini /srv/hedera-61850-gateway/

ENTRYPOINT ["java", "-jar", "/srv/hedera-61850-gateway/gateway-app-0.1-SNAPSHOT-all.jar"]