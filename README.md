# meshtastic network sim in a xrdp ubuntu 25 docker container 
the official mestastic simulator doesnt work very well so i tried to put it into the docker container from my qemu & vice emulator project. This is to test my python discord bot for meshtastic without involving real devices
<br>
<img src="https://github.com/user-attachments/assets/b81e6036-8728-4885-ac1f-dc497fde9013" width="75%" alt="Screenshot">

- ubuntu 25.04 + XRDP for remote desktop
- has all the apt-get and pip packages installed for meshtastic 
- use remote desktop to access docker container to run everything so its network-isolated
<br>
<br>


using it:
```
git clone https://github.com/xp5-org/meshtastic-sim-testrunner.git
cd meshtastic-sim-testrunner
docker build -t meshsim .
docker run --rm -p 3389:3389 -e USERNAME=user -e USERPASSWORD=a -v /your/shared_dir:/path_inside_container meshsim:latest
```

<br>
<br>

using python inside the container:
- todo : fix the permissions problem
```
source /opt/venv/bin/activate
sudo chmod -R 0775 /opt/venv/
```
<br>
<br>


this one i ran up to 200, its right on the edge of channel overutilisation with default settings maybe 300 is the max on a single default channel setting and idle clients

```
for i in {1..200} ;do     xterm -T "Node 440$i" -e /home/user/Meshtasticator/Meshtasticator-device/.pio/build/native/program --port 440$i -e &     sleep 1; done
```

# meshterminal.py --port 4403
- connects to 127.0.0.1 at port number, lists node info, and 
- viewing one of the mesh sim hosts, 200 nodes active

<img width="382" height="912" alt="Screenshot 2025-08-24 at 6 40 48 PM" src="https://github.com/user-attachments/assets/707ee810-f19a-4c5a-9c4e-964f2eeb9abd">

 
```
(venv) user@cfaa3940fc8d:/meshpy$ meshtastic --host 127.0.0.1:4403 --set mqtt.encryption_enabled false
Connected to radio
Set mqtt.encryption_enabled to false
Writing modified preferences to device
Writing mqtt configuration to device
```


also working on an mqtt broker to simulate forwarding messages between pools of different meshes 


```
(venv) user@cfaa3940fc8d:/meshpy$ gcc -std=c89 -Wall relay.c -o mqttrelay
(venv) user@cfaa3940fc8d:/meshpy$ ./mqttrelay 
Minimal MQTT broker running on port 1883
[CONNECT] New client connected, slot 0
[CONNECT] Client slot 0 sent CONNECT
[CONNACK] Sent to client slot 0
[CONNECT] New client connected, slot 1
[CONNECT] Client slot 1 sent CONNECT
[CONNACK] Sent to client slot 1
```

missing quite a bit and needs work. if i can find something else to read & send the mqtt messages, specifically need to look at the tcp sender and decide where to route it so i can simulate different hop-groups of meshes routed through mqtt
