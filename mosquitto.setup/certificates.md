CA:

openssl req -new -x509 -days 3650 -extensions v3_ca -keyout ca.key -out ca.crt

server:

openssl genrsa -out server.key 2048
openssl req -new -out server.csr -key server.key
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 3650

client:

openssl genrsa  -out client1.key 2048
openssl req -out client1.csr -key client1.key -new
openssl x509 -req -in client1.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client1.crt -days 3650

convert:

openssl x509 -outform der -in client1.crt -out data2DA3/client1.der
openssl rsa -outform der -in client1.key -out data2DA3/client1key.der
openssl x509 -outform der -in ca.crt -out ca.der

info:

openssl x509 -in server.crt -inform pem -text -noout

verify:

openssl verify -verbose -CAfile ca.crt server.crt
openssl verify -verbose -CAfile ca.crt client1.crt
