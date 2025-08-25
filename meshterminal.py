import json
import threading
import time
from pubsub import pub
from meshtastic.tcp_interface import TCPInterface


host='127.0.0.1'




class MeshSession:
    def __init__(self, host, portnum):
        self.node_ip = host
        self.node_portnum = int(portnum)
        self.node_list = []
        self.node_list_lock = threading.Lock()
        self.my_node_id = None

        try:
            self.interface = TCPInterface(hostname=self.node_ip, portNumber=self.node_portnum)
        except OSError as e:
            print(f"ERROR: Could not connect to {self.node_ip}:{self.node_portnum}: {e}")
            exit(1)

        pub.subscribe(self.on_receive, "meshtastic.receive")





    def fetch_nodes(self):
        from pprint import pprint
        # debug: dump entire nodes dict
        #print("\n=== RAW NODE DATA ===")
        #for node_id, node in self.interface.nodes.items():
        #    pprint({node_id: node})
        devices = []
        for node_id, node in self.interface.nodes.items():
            user = node.get("user", {})
            metrics = node.get("deviceMetrics", {})

            long_name = user.get("longName", "Unknown")
            role = user.get("role", "none")
            hops = node.get("hopsAway", "?")
            snr = metrics.get("snr", None)

            # if hops count val is ? it means malformed or invalid?
            # lets set it to 99 so we can sort them by themselves as errors
            # without changing how an integer is handled here 
            if hops == "?":
                hops_val = 99
            else:
                try:
                    hops_val = int(hops)
                except (TypeError, ValueError):
                    hops_val = 99

            info = {
                "longName": long_name,
                "role": role,
                "hopsAway": hops_val,
                "snr": snr
            }
            devices.append((node_id, info))

        # also set to 9999 if not present or none?
        devices.sort(key=lambda item: item[1].get("hopsAway", 9999))

        with self.node_list_lock:
            self.node_list[:] = devices
            self.my_node_id = next(
                (nid for nid, info in devices if info.get("role") == "self"),
                None
            )

        print("\nNode list refreshed:")
        for node_id, info in devices:
            print(f"{node_id} ({info['longName']}) - hops: {info['hopsAway']}, "
                  f"SNR: {info['snr']}, role: {info['role']}")

    def on_receive(self, packet, interface):
        try:
            decoded_packet = packet.get('decoded', {})
            if decoded_packet.get('portnum') == 'TEXT_MESSAGE_APP':
                message = decoded_packet['payload'].decode('utf-8')
                fromnum = packet['fromId']

                with self.node_list_lock:
                    info = next((info for nid, info in self.node_list if nid == fromnum), {})
                shortname = info.get('longName', 'Unknown')
                hops = info.get('hopsAway', '?')

                to_id = packet.get('toId') or decoded_packet.get('to')
                if to_id is None or to_id == '^all':
                    prefix = "C]"
                elif to_id == self.my_node_id:
                    prefix = "D]"
                else:
                    prefix = "UNK]"

                print(f"H{hops} {prefix} {shortname}: {message}")
        except (KeyError, UnicodeDecodeError):
            pass



    def listen_messages(self):
        print("Listening for messages...")
        try:
            while True:
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("Listener stopped.")
            self.interface.close()




def main():
    import argparse

    parser = argparse.ArgumentParser(description="Meshtastic TCP client")
    parser.add_argument("--port", type=int, default=4003, help="TCP port to connect to (default: 4003)")
    args = parser.parse_args()

    print(f"connecting to {host} {args.port}")

    session = MeshSession(host, args.port)
    session.fetch_nodes()

    listener_thread = threading.Thread(target=session.listen_messages, daemon=True)
    listener_thread.start()

    print("Type messages to send. Press ']' first and Enter to refresh node list.")

    while True:
        try:
            inp = input()
            if not inp:
                continue
            if inp[0] == ']':
                session.fetch_nodes()
            else:
                session.interface.sendText(inp)
        except KeyboardInterrupt:
            print("Exiting.")
            break



if __name__ == "__main__":
    main()
