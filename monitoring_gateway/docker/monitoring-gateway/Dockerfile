FROM alpine:3.8

ENV PROCESS_USER mntgwusr
ENV PROCESS_UID 7000
ENV PROCESS_GROUP mntgwusr
ENV PROCESS_GID 8000

RUN set -x \
    && addgroup $PROCESS_USER \
    && adduser -u $PROCESS_UID -G $PROCESS_USER -H -D -s /bin/bash $PROCESS_USER \
    && addgroup -g $PROCESS_GID $PROCESS_GROUP

ADD monitoring-gateway /
RUN chown $PROCESS_USER:$PROCESS_GROUP /monitoring-gateway
RUN chmod +x /monitoring-gateway

ENTRYPOINT ["/monitoring-gateway", "-config-dir=/config"]
