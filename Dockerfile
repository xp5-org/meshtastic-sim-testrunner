FROM ubuntu:25.04

ENV TZ=UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        xfce4 \
        xfce4-clipman-plugin \
        xfce4-cpugraph-plugin \
        xfce4-netload-plugin \
        xserver-xorg-legacy \
        xdg-utils \
        dbus-x11 \
        xfce4-screenshooter \
        xfce4-taskmanager \
        xfce4-terminal \
        xfce4-xkb-plugin \
        xorgxrdp \
        xrdp \
        sudo \
        wget \
        curl \
        bzip2 \
        python3 \
        python3-pip \
        python3-venv \
        build-essential \
        xterm \
        git \
        vim \
        pkg-config \
        libusb-1.0-0-dev \
        libuv1-dev \
        libgpiod-dev \
        libbluetooth-dev \
        libi2c-dev \
        libyaml-cpp-dev \
        cmake \
        ninja-build \
        python3-dev \
        libudev-dev \
        libssl-dev \
        libffi-dev \
        libncurses5-dev \
        libncursesw5-dev \
        zlib1g-dev \
        libsqlite3-dev \
        libreadline-dev \
        libbz2-dev \
        liblzma-dev \
        libpng-dev \
        libjpeg-dev \
        libfreetype6-dev \
        linux-headers-generic \
        ncurses-dev \
        xdotool \
        python3-tk \
        unzip && \
    apt-get remove -y light-locker xscreensaver && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/*



# Fix XRDP/X11 setup
RUN mkdir -p /var/run/dbus && \
    cp /etc/X11/xrdp/xorg.conf /etc/X11 || true && \
    sed -i "s/console/anybody/g" /etc/X11/Xwrapper.config && \
    sed -i "s|xrdp/xorg|xorg|g" /etc/xrdp/sesman.ini && \
    echo "xfce4-session" >> /etc/skel/.Xsession





# -----------------------
# Setup Meshtastic environment
# -----------------------
WORKDIR /home/user
RUN git clone https://github.com/meshtastic/Meshtasticator.git
RUN mkdir -p /home/user/Meshtasticator/Meshtasticator-device && \
    git clone https://github.com/meshtastic/firmware.git /home/user/Meshtasticator/Meshtasticator-device

WORKDIR /home/user/Meshtasticator/Meshtasticator-device

RUN python3 -m venv /opt/venv && \
    /opt/venv/bin/pip install --upgrade pip && \
    /opt/venv/bin/pip install platformio PyYAML matplotlib meshtastic scipy && \
    chgrp -R users /opt/venv && chmod -R g+rwX /opt/venv && \
    find /opt/venv -type d -exec chmod g+s {} \;

ENV VENV_PATH=/opt/venv
ENV PATH="$VENV_PATH/bin:$PATH"

RUN /opt/venv/bin/pio run -e native




# -----------------------
# Setup runtime
# -----------------------
WORKDIR /root
COPY entrypoint.sh /app/
RUN chmod +x /app/entrypoint.sh

EXPOSE 3389 8080
ENTRYPOINT ["/app/entrypoint.sh"]