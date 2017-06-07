#use default image
FROM postgres:latest
#Author
MAINTAINER troy "liuchunwei@bubi.cn"

#environments
ENV POSTGRES_PASSWORD root
ENV BUBI_HOME /usr/blockchain
ENV PATH $PATH:$BUBI_HOME/bin

#create directory, then copy files
RUN mkdir -p $BUBI_HOME
ADD blockchain $BUBI_HOME

#add entrypoint.sh
COPY bubi-entrypoint.sh /bubi-entrypoint.sh
RUN chmod 777 /bubi-entrypoint.sh

#hack default entrypoint of postgres
COPY postgres-entrypoint.sh /docker-entrypoint.sh
RUN chmod 777 /docker-entrypoint.sh


#expose 19333
EXPOSE 19333


#bubi entrypoint
ENTRYPOINT /bubi-entrypoint.sh 
