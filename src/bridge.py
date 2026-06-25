import evdev
import zmq
import json

ADDRESS = "tcp://localhost:6767"

print("Initialising input bridge")

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://localhost:6767")

input_handle = evdev.UInput()

print(f"Listening on {ADDRESS} for requests...")

while True:
    try:
        message: dict = json.loads(socket.recv())
    except json.JSONDecodeError:
        print("Recieved invalid packet")
        socket.send(b"-1") # invalid request
        continue
    except Exception as e:
        print(f"A fatal error occured: {e}")
        break

    try:
        if message["type"] == "input":
            print("Recieved input request!")
            if len(message["keys"]) > 0:
                for key in message["keys"]:
                    input_handle.write(evdev.ecodes.EV_KEY, int(key), 1)
                    print(f"Pressed key: {key}")

                input_handle.syn()

                for key in message["keys"]:
                    input_handle.write(evdev.ecodes.EV_KEY, int(key), 0)
                
                socket.send(b"1") # recieved, success
                print("Request succeeded!")
            else:
                socket.send(b"0") # recieved, failed
                print("Request failed!")
        elif message["type"] == "shutdown":
            print("Recieved shutdown request!")
            socket.send(b"1") # recieved, success
            
            break
        else:
            print(f"Recieved invalid request: {message["type"]}")
            socket.send(b"-1") # invalid request
    except ValueError:
        print("Recieved invalid packet")
        socket.send(b"-1")
        continue
    except Exception as e:
        print(f"A fatal error occured: {e}")
        break

print("Closing connection and handles...")

input_handle.close()
socket.close()
context.term()

print("Input bridge shutdown :)")