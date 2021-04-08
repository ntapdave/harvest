FROM phusion/baseimage:bionic-1.0.0


#install packages required by graphite-statsd & grafana
RUN apt-get -y update\
 && apt-get -y -o Dpkg::Options::="--force-confdef" upgrade\
 && apt-get -y --force-yes install vim\
 python-dev\
 python-flup\
 python-pip\
 python-ldap\
 expect\
 git\
 pkg-config\
 && apt-get clean

#install packages required by NetApp Harvest
RUN apt-get -y update\
 && apt-get -y --force-yes install libjson-perl\
 libwww-perl\
 libxml-parser-perl\
 liblwp-protocol-https-perl\
 libexcel-writer-xlsx-perl\
 && apt-get clean

# install and configure NetApp Harvest
COPY conf/opt/netapp-harvest/netapp-harvest_1.6_all.deb /tmp
COPY conf/opt/netapp-harvest/cdot*.conf /opt/netapp-harvest/template/
COPY conf/opt/netapp-harvest/sdk9.8p1-perl/* /opt/netapp-harvest/lib/
COPY conf/opt/netapp-harvest/netapp-harvest.conf /opt/netapp-harvest/netapp-harvest.conf
RUN dpkg -i /tmp/netapp-harvest_1.6_all.deb

ADD conf/etc/service/netapp-harvest/run /etc/service/netapp-harvest/run

# default conf setup
#ADD conf/etc/my_init.d/01_conf_init.sh /etc/my_init.d/01_conf_init.sh

# cleanup
RUN apt-get clean\
 && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# defaults
EXPOSE 80 2003-2004 2023-2024 8125 8125/udp 8126
WORKDIR /
ENV HOME /root

CMD ["/sbin/my_init"]
