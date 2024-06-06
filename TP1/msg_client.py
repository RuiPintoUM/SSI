# Código baseado em https://docs.python.org/3.6/library/asyncio-stream.html#tcp-echo-client-using-streams
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.hazmat.primitives.asymmetric import dh
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.primitives import hashes
import datetime
import asyncio
import os, sys
import socket
import aux

conn_port = 8443
max_msg_size = 9999

p = 0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF
g = 2

pn = dh.DHParameterNumbers(p,g)
Parameters = pn.parameters()

class Client:
    """ Classe que implementa a funcionalidade de um CLIENTE. """
    def __init__(self, sckt=None, fname="userdata.p12"):
        """ Construtor da classe. """
        self.sckt = sckt
        self.msg_cnt = 0
        self.pk_DH = Parameters.generate_private_key()
        self.pk_RSA, self.cert, self.ca_cert = aux.get_userdata(fname)
        self.pub_Server = None
        self.aergcm = None
        self.getmsg = False

    def load_private_key(self, filename):
        """Carrega a chave privada do arquivo."""
        with open(filename, 'rb') as key_file:
            key_data = key_file.read()

        private_key = serialization.load_pem_private_key(
            key_data, 
            backend= default_backend()
        )
        return private_key

    def process(self, msg=b""):
        """ Processa uma mensagem (`bytestring`) enviada pelo SERVIDOR.
            Retorna a mensagem a transmitir como resposta (`None` para
            finalizar ligação) """
        self.msg_cnt +=1

        nonce = msg[:12]
        ciphertext = msg[12:] 
        #
        # ALTERAR AQUI COMPORTAMENTO DO CLIENTE
        #
        
        if self.msg_cnt < 3:
            if self.msg_cnt == 1:
                pub_key = self.pk_DH.public_key()
                new_msg = pub_key.public_bytes(
                            encoding=serialization.Encoding.PEM,
                            format=serialization.PublicFormat.SubjectPublicKeyInfo
                            )

            if self.msg_cnt == 2:
                pair_PubKey_SignServer, certServer = aux.unpair(msg)

                pubkey_Server_serialized, signServer = aux.unpair(pair_PubKey_SignServer)
                
                deserialized_cert_server = aux.deserialize_certificate(certServer)

                if(aux.valida_cert(deserialized_cert_server, self.ca_cert)):
                    try:
                        # Deserialize the public key from the received message
                        pubkey_ServerDH = serialization.load_pem_public_key(
                            pubkey_Server_serialized,
                            backend=default_backend()
                        )
                    except Exception as e:
                        print(f"Error deserializing public key: {e}")
                        return None

                    self.pub_Server = pubkey_ServerDH

                    pubkey_ServerRSA = deserialized_cert_server.public_key()

                    pub_key = self.pk_DH.public_key()
                    pubKey_Client_serialized = pub_key.public_bytes(
                                                    encoding=serialization.Encoding.PEM,
                                                    format=serialization.PublicFormat.SubjectPublicKeyInfo
                                                )
                    
                    pair_pubkeys = aux.mkpair(pubkey_Server_serialized,pubKey_Client_serialized)

                    pubkey_ServerRSA.verify(
                        signature=signServer,
                        data= pair_pubkeys,
                        padding=padding.PSS(
                            mgf=padding.MGF1(hashes.SHA256()),
                            salt_length=padding.PSS.MAX_LENGTH
                        ),
                        algorithm=hashes.SHA256()
                    )

                    signClient = self.pk_RSA.sign(
                            pair_pubkeys,
                            padding.PSS(
                                mgf=padding.MGF1(hashes.SHA256()),
                                salt_length=padding.PSS.MAX_LENGTH
                            ),
                            hashes.SHA256()
                        )
                    
                    cert_serialised = aux.serialize_certificate(self.cert)
                    
                    new_msg = aux.mkpair(signClient, cert_serialised)

                    shared_key = self.pk_DH.exchange(pubkey_ServerDH)

                    derived_key = HKDF(
                        algorithm=hashes.SHA256(),
                        length=32,
                        salt=None,
                        info=b'handshake data',
                    ).derive(shared_key)

                    self.aesgcm = AESGCM(derived_key)

                else:
                    print("MSG RELAY SERVICE: verification error!")

        else:
            if(not self.getmsg):
                plaintext = self.aesgcm.decrypt(nonce, ciphertext, None)
                txt = plaintext.decode()
                if txt == "getmsg_error":
                    print("Mensagem não encontrada!")
                else:
                    print('\nReceived (%d): %r' % (self.msg_cnt , txt))

            else:
                self.getmsg = False
                pair_MesIfSig_Cert_encoded = self.aesgcm.decrypt(nonce, ciphertext, None)
                pair_MesIfSig_Cert = pair_MesIfSig_Cert_encoded

                pair_MsgInfo_Sign, certSender_encrypted = aux.unpair(pair_MesIfSig_Cert)

                cert_Sender = aux.deserialize_certificate(certSender_encrypted)
                
                if (aux.valida_cert(cert_Sender, self.ca_cert)):
                    msgInfo, sign = aux.unpair(pair_MsgInfo_Sign)
                    pubKey_SenderRSA = cert_Sender.public_key()
                    
                    try:
                        pubKey_SenderRSA.verify( sign,
                            msgInfo,        
                            padding.PSS(
                            mgf=padding.MGF1(hashes.SHA256()),
                            salt_length=padding.PSS.MAX_LENGTH
                            ),
                            hashes.SHA256()
                    )
                    except Exception as e:
                        print("MSG RELAY SERVICE: verification error!")
                        
                    uid_receiver, content = aux.unpair(msgInfo)
                    certInfo = aux.parseCert(self.cert)

                    if(uid_receiver.decode() == certInfo["pseudonym"]):

                        print(content.decode())

                    else:
                        print("MSG RELAY SERVICE: verification error!")
                else:
                    print("MSG RELAY SERVICE: verification error!")

            print('Input message to send (empty to finish):')

            command = input()
            command_splited = command.split()
            new_msg = ""
            new_msg_command = ""

            match command_splited[0]:
                case "-user":
                    self.path_clientInfo = command_splited[1]
                    print("User Info foi atualizado para: %s", command_splited[1])
                    new_msg_command = "help" 

                case "send":
                    content = sys.stdin.read(1000)

                    uid_receiver = command_splited[1]

                    mesage_info = aux.mkpair(uid_receiver.encode(), content.encode())
                    
                    sign_MesageInfo = self.pk_RSA.sign(
                                        mesage_info,
                                        padding.PSS(
                                            mgf=padding.MGF1(hashes.SHA256()),
                                            salt_length=padding.PSS.MAX_LENGTH
                                        ),
                                        hashes.SHA256()
                                    )
                    
                    certSender_encrypted = aux.serialize_certificate(self.cert)
                    
                    new_msg_command = command

                    new_msg =  aux.mkpair(new_msg_command.encode(), aux.mkpair( aux.mkpair(mesage_info, sign_MesageInfo), certSender_encrypted))

                case "askqueue":
                    new_msg_command = command
                    certSender_info = aux.parseCert(self.cert)
                    
                    new_msg = aux.mkpair(new_msg_command.encode(), certSender_info["pseudonym"].encode()) 

                case "getmsg":
                    new_msg_command = command
                    certSender_info = aux.parseCert(self.cert)
                    self.getmsg = True

                    new_msg = aux.mkpair(new_msg_command.encode(), certSender_info["pseudonym"].encode()) 

                case "help":
                    print("--------------------------------------------")
                    print("      Comandos disponíveis\n")


                    print("--> -user <FNAME>")
                    print("Alterar o ficheiro com a client info")
                    print("<FNAME>: nome do ficheiro\n")

                    print("--> askqueue")
                    print("Devolve lista de mensagens por ler\n")

                    print("--> getmsg <NUM>")
                    print("Ler mensagem")
                    print("<NUM>: número da mensagem\n")
                    print("--------------------------------------------")
                    new_msg_command = "help"
                    certSender_info = aux.parseCert(self.cert)
                    new_msg = aux.mkpair(new_msg_command.encode(), certSender_info["pseudonym"].encode()) 
                    new_msg = new_msg_command.encode()

                case default:
                    print("MSG RELAY SERVICE: verification error!\n", file=sys.stderr)
                    print("--------------------------------------------")
                    print("      Comandos disponíveis\n")


                    print("--> -user <FNAME>")
                    print("Alterar o ficheiro com a client info")
                    print("<FNAME>: nome do ficheiro\n")

                    print("--> askqueue")
                    print("Devolve lista de mensagens por ler\n")

                    print("--> getmsg <NUM>")
                    print("Ler mensagem")
                    print("<NUM>: número da mensagem\n")
                    print("--------------------------------------------")
                    new_msg_command = "invalid_command"
                    new_msg = new_msg_command.encode()


            if new_msg:
                nonce = os.urandom(12)
                ciphertext = self.aesgcm.encrypt(nonce, new_msg, None)
                new_msg = nonce + ciphertext

        return new_msg if len(new_msg)>0 or self.msg_cnt < 3 else None


#
#
# Funcionalidade Cliente/Servidor
#
# obs: não deverá ser necessário alterar o que se segue
#


async def tcp_echo_client():
    reader, writer = await asyncio.open_connection('127.0.0.1', conn_port)
    addr = writer.get_extra_info('peername')
    if len(sys.argv) >= 3:
        fname = sys.argv[2]
    else:
        fname = "MSG_CLI1.p12"
        
    client = Client(addr, fname)
    msg = client.process()
    while msg:
        writer.write(msg)
        msg = await reader.read(max_msg_size)

        if msg :
            msg = client.process(msg)
        else:
            break
    writer.write(b'\n')
    print('Socket closed!')
    writer.close()

def run_client():
    loop = asyncio.get_event_loop()
    loop.run_until_complete(tcp_echo_client())

def main():
    run_client()
    
if __name__ == "__main__":
    main()
