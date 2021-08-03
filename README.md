# Independa Instructions
* Used Mac OSX
* Note: Requires having installed CMAKE, openssl

- Use below to clone your local repo.
```
git clone --recurse-submodules https://github.com/Independa/socketioLG.git
```
- Run 
```cmake -DTARGET=sioclient_tls ./```

- To build socket.io client app run
```
g++ -std=c++11  main.cpp ./src/sio_client.cpp ./src/internal/sio_client_impl.cpp ./src/internal/sio_packet.cpp ./src/sio_socket.cpp -o main -I ./lib/websocketpp -I ./lib/rapidjson/include -I ./src/libsioclient_tls.a -lssl -lcrypto -I /usr/local/opt/openssl@1.1/include -L /usr/local/opt/openssl@1.1/lib -lcurl
```
- To run client
```
./main
```

## License
Based on https://github.com/socketio/socket.io-client-cpp
MIT
