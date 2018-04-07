#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <gpgme.h>
#include <locale.h>
#include <stdlib.h>
#include <errno.h>
#include <QDebug>
#include <stdio.h>

#define detectError(err)                                    \
    if (err) {                                              \
        fprintf (stderr, "%s:%d: %s: %s\n",                 \
                __FILE__, __LINE__, gpgme_strsource (err),  \
                gpgme_strerror (err));                      \
            exit (1);                                       \
        }                                                   \


class Encryption
{
public:
    Encryption();

    gpgme_key_t getKey(char *pattern, int isPrivateKey);
    void setServerKey(gpgme_key_t key);

    void printKeys(gpgme_key_t key);
    int listKeys(gpgme_ctx_t ctx, gpgme_error_t err, gpgme_key_t key, int onlyPrivateFlag);
    gpgme_genkey_result_t genKey(const char *parms);
    void exportKey(gpgme_key_t key, const char *outputFileName);
    gpgme_import_result_t importKey(const char *inFileName);
    void encrypt(gpgme_ctx_t ctx, gpgme_error_t err, gpgme_key_t recv,
                 const char *inputFileName, const char *outputFileName);
    void encryptSign(gpgme_key_t signKey, gpgme_key_t recpKey,
                     const char *inputFileName,const char *outputFileName);
    void decrypt(gpgme_ctx_t ctx, gpgme_error_t err, const char *inputFileName,
                 const char *outputFileName);
    bool decryptVerify(const char *inputFileName,
                 const char *outputFileName);

    gpgme_key_t serverKey = nullptr;

private:
    gpgme_ctx_t ctx;
    gpgme_error_t err;
    QByteArray activeUserPubKey;


};


#endif // ENCRYPTION_H
