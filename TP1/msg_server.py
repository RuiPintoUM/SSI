# Código baseado em https://docs.python.org/3.6/library/asyncio-stream.html#tcp-echo-client-using-streams
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import dh
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.primitives import hashes
from collections import deque 
import datetime
import sys
import asyncio
import os
import aux as auxiliar

conn_cnt = 0
conn_port = 8443
max_msg_size = 9999

UsersQueues = {}

p = 0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF
g = 2

pn = dh.DHParameterNumbers(p,g)
Parameters = pn.parameters()

class Message:
    def __init__(self, subject, mesage_info_info_signature, msg_number, timestamp, sender, certSender):
        self.number = msg_number
        self.subject = subject
        self.pair_mesage_info_signature = mesage_info_info_signature
        self.read_status = False
        self.timestamp = timestamp
        self.sender = sender
        self.certSender = certSender

class ServerWorker(object):
    """ Classe que implementa a funcionalidade do SERVIDOR. """
    def __init__(self, cnt, addr=None):
        """ Construtor da classe. """
        self.id = cnt
        self.addr = addr
        self.msg_cnt = 0
        self.pk_DH = Parameters.generate_private_key()
        self.pk_RSA , self.server_cert, self.ca_cert = auxiliar.get_userdata("MSG_SERVER.p12")
        self.pubKey_ClientDH = None
        self.aesgcm = None

    def process(self, msg):
        """ Processa uma mensagem (`bytestring`) enviada pelo CLIENTE.
            Retorna a mensagem a transmitir como resposta (`None` para
            finalizar ligação) """
        self.msg_cnt += 1

        #
        # ALTERAR AQUI COMPORTAMENTO DO SERVIDOR
        #

        if self.msg_cnt < 3:
            if self.msg_cnt == 1:
                try:
                    pub_keyClient = serialization.load_pem_public_key(
                        msg,
                        backend=default_backend()
                    )
                except Exception as e:
                    print(f"Error deserializing public key: {e}")
                    return None

                self.pubKey_ClientDH = pub_keyClient

                pub_key = self.pk_DH.public_key()
                pub_key_serialized = pub_key.public_bytes(
                                 encoding=serialization.Encoding.PEM,
                                 format=serialization.PublicFormat.SubjectPublicKeyInfo
                                 )

                pair_pubkeys = auxiliar.mkpair(pub_key_serialized,msg)
                signServer = self.pk_RSA.sign(
                            pair_pubkeys,
                            padding.PSS(
                                mgf=padding.MGF1(hashes.SHA256()),
                                salt_length=padding.PSS.MAX_LENGTH
                            ),
                            hashes.SHA256()
                        )
                
                server_cert_serialized = auxiliar.serialize_certificate(self.server_cert)

                new_msg = auxiliar.mkpair(auxiliar.mkpair(pub_key_serialized, signServer), server_cert_serialized)

            if self.msg_cnt == 2:
                signClient, certClient_serialized = auxiliar.unpair(msg)

                certClient = auxiliar.deserialize_certificate(certClient_serialized)

                pub_key = self.pk_DH.public_key()
                pub_key_serialized_Server = pub_key.public_bytes(
                                    encoding=serialization.Encoding.PEM,
                                    format=serialization.PublicFormat.SubjectPublicKeyInfo
                                    )
                
                pub_key_serialized_Client = self.pubKey_ClientDH.public_bytes(
                                            encoding=serialization.Encoding.PEM,
                                            format=serialization.PublicFormat.SubjectPublicKeyInfo
                                            )
                
                pair_pubkeys = auxiliar.mkpair(pub_key_serialized_Server,pub_key_serialized_Client)

                if(auxiliar.valida_cert(certClient, self.ca_cert)):
                    pubkey_ClientRSA = certClient.public_key()

                    pubkey_ClientRSA.verify( signClient,
                            pair_pubkeys,        
                            padding.PSS(
                            mgf=padding.MGF1(hashes.SHA256()),
                            salt_length=padding.PSS.MAX_LENGTH
                            ),
                            hashes.SHA256()
                    )

                    shared_key = self.pk_DH.exchange(self.pubKey_ClientDH)

                    derived_key = HKDF(
                        algorithm=hashes.SHA256(),
                        length=32,
                        salt=None,
                        info=b'handshake data',
                    ).derive(shared_key)

                    self.aesgcm = AESGCM(derived_key)

                    cert_infos = auxiliar.parseCert(certClient)

                    txt = "Sucesso!"
                    new_msg = txt.upper().encode()
                    nonce = os.urandom(12)
                    ciphertext = self.aesgcm.encrypt(nonce, new_msg, None)
                    new_msg = nonce + ciphertext

                else:
                    print("MSG RELAY SERVICE: verification error!")
        
        else:
            nonce = msg[:12]
            ciphertext = msg[12:]

            pair_TxtCert = self.aesgcm.decrypt(nonce, ciphertext, None)
            plaintext, clientStuff = auxiliar.unpair(pair_TxtCert)
            txt = plaintext.decode()

            print('%d : %r' % (self.id, txt))
            with open("server_log.txt", 'a') as log:
                log.write(f"Received: [{self.id}]: {txt}\n")

            txt_splited = txt.split()
            response = "sem comando"

            match txt_splited[0]:
                case "send":
                    if txt_splited[1] not in UsersQueues:
                        UsersQueues[txt_splited[1]] = deque()

                    timestamp = auxiliar.get_timestamp()

                    pair_mesageInfo_Signature, certSender_encrypted = auxiliar.unpair(clientStuff)

                    certSender = auxiliar.deserialize_certificate(certSender_encrypted)

                    certSender_info = auxiliar.parseCert(certSender)

                    new_msg = Message(txt_splited[2], pair_mesageInfo_Signature, len(UsersQueues[txt_splited[1]]), timestamp, certSender_info["commonName"], certSender)

                    UsersQueues[txt_splited[1]].append(new_msg)

                    response = f'Mensagem enviada para {txt_splited[1]} com sucesso'

                    new_msg = response.encode()

                case "askqueue":
                    response = "As suas mensagens são:\n"
                    uid_client = clientStuff.decode()

                    for aux in UsersQueues[uid_client]:
                        if not aux.read_status:
                            response = response + f"{aux.number}:{aux.sender}:{aux.timestamp}:{aux.subject}\n"

                    new_msg = response.encode()

                case "getmsg":
                    uid_client = clientStuff.decode()
                    
                    for aux in UsersQueues[uid_client]:
                        if aux.number == int(txt_splited[1]):
                            certSender_encrypted = auxiliar.serialize_certificate(aux.certSender) 
                            mesageAndSec = auxiliar.mkpair(aux.pair_mesage_info_signature, certSender_encrypted)

                            response = mesageAndSec
                            aux.read_status = True 

                            new_msg = response
                            
                case "invalid_command":
                    response = "invalid_command_response"
                    new_msg = response.encode()
                
                case default:
                    response = "help"
                    new_msg = response.encode()

            with open("server_log.txt", 'a') as log:
                log.write(f"Sent: {response}\n")
                log.write("________________\n")
                
            nonce = os.urandom(12)
            ciphertext = self.aesgcm.encrypt(nonce, new_msg, None)
            new_msg = nonce + ciphertext

        return new_msg if len(new_msg)>0 else None


#
#
# Funcionalidade Cliente/Servidor
#
# obs: não deverá ser necessário alterar o que se segue
#


async def handle_echo(reader, writer):
    global conn_cnt
    conn_cnt +=1
    addr = writer.get_extra_info('peername')
    srvwrk = ServerWorker(conn_cnt, addr)
    data = await reader.read(max_msg_size)
    while True:
        if not data: continue
        if data[:1]==b'\n': break
        data = srvwrk.process(data)
        if not data: break
        writer.write(data)
        await writer.drain()
        data = await reader.read(max_msg_size)
    print("[%d]" % srvwrk.id)
    writer.close()


def run_server():
    with open("server_log.txt", 'w') as log:
        log.truncate(0)
    loop = asyncio.new_event_loop()
    coro = asyncio.start_server(handle_echo, '127.0.0.1', conn_port)
    server = loop.run_until_complete(coro)
    # Serve requests until Ctrl+C is pressed
    print('Serving on {}'.format(server.sockets[0].getsockname()))
    print('  (type ^C to finish)\n')
    try:
        loop.run_forever()
    except KeyboardInterrupt:
        pass
    # Close the server
    server.close()
    loop.run_until_complete(server.wait_closed())
    loop.close()
    print('\nFINISHED!')

run_server()
