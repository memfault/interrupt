import os, sys
import binascii
from Crypto.Cipher import AES
from Crypto.Util import Counter

# We assume that the key was securely shared beforehand
# Asymmetric key
mykey = "58:3b:f4:90:1f:62:35:10:a1:40:8e:8b:e7:bd:6f:5a"
# Initialization vector Nonce
myiv  = "f8:24"

BYTES_READ = 4

class  EncryptionCTR:
    def __init__(self, key, iv ):
        self.key = binascii.unhexlify(key.replace(":", ""))
        self.iv  = binascii.unhexlify(iv.replace(":", ""))
        self.count_ctr = Counter.new(112, prefix=self.iv, little_endian=False, initial_value=0) 
        self.cipherObject = AES.new(self.key, AES.MODE_CTR, counter=self.count_ctr)

    def EncryptCtr(self, plaintext):
        return self.cipherObject.encrypt(bytes(plaintext))

    def SwapLittleEndian(self,plaintext):
        temp_swap = [bytes]*BYTES_READ
        temp_swap[0] = plaintext[3]
        temp_swap[1] = plaintext[2]
        temp_swap[2] = plaintext[1]
        temp_swap[3] = plaintext[0]
        return temp_swap

class BinaryFile:
    def __init__(self, nameFile, mode):
        self.openFileName = nameFile
        try:
            self.ObjectFile = open(self.openFileName , mode)
        except IOError:
            print("Cannot open binary file \n")
            print("Make sure it is in the same folder as the script\r\n")
            sys.exit(1)

    def CloseFile(self):
        self.ObjectFile.close()

    def ReadFile(self, numberBytes):
        return self.ObjectFile.read(numberBytes)

    def WriteFile(self, dataBytes):
        self.ObjectFile.write(bytes(dataBytes))

    def SizeFile(self):
        return os.path.getsize( self.openFileName )


#Entry point the Script
def main():
    print(__name__)
    
    PlainFirmware  = BinaryFile(sys.argv[1],'rb')
    CipherFirmware = BinaryFile(sys.argv[2],'wb') 
    CrytoFirmware  = EncryptionCTR(mykey,myiv)

    bytes_remaining = 0
    bytes_so_far_cryted = 0

    BinaryFileSize = PlainFirmware.SizeFile()
    bytes_remaining = BinaryFileSize - bytes_so_far_cryted
    print("Bytes to encrypt :{}", bytes_remaining )

    while(bytes_remaining):
        Plainbytes = PlainFirmware.ReadFile(BYTES_READ)
        PlainbytesLittle = CrytoFirmware.SwapLittleEndian(Plainbytes)
        Cipherbytes = CrytoFirmware.EncryptCtr(PlainbytesLittle)
        CipherbytesLittle = CrytoFirmware.SwapLittleEndian(Cipherbytes)
        CipherFirmware.WriteFile(CipherbytesLittle)

        print( PlainbytesLittle )
        print( CipherbytesLittle )
        
        bytes_so_far_cryted += BYTES_READ
        bytes_remaining = BinaryFileSize - bytes_so_far_cryted
        if( bytes_remaining <= 0 ): bytes_remaining = 0
        print( bytes_remaining )
        
    PlainFirmware.CloseFile()
    CipherFirmware.CloseFile()


main()

