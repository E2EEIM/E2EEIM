/****************************************************************************
** Copyright (C) 2018  Krittitam Juksukit
** This file is part of E2EEIM.
**
** E2EEIM is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License
** version 3 as published by the Free Software Foundation.
**
** E2EEIM is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with E2EEIM.  If not, see <http://www.gnu.org/licenses/>.
**
*******************************************************************************/
#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <QCoreApplication>

#include <gpgme.h>
#include <locale.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#ifdef QT_DEBUG
#define LOG true
#else
#define LOG false
#endif

#define detectError(err)                                    \
    if (err) {                                              \
        fprintf (stderr, "%s:%d: %s: %s\n",                 \
                __FILE__, __LINE__, gpgme_strsource (err),  \
                gpgme_strerror (err));                      \
            exit (1);                                       \
        }                                                   \


void printKeys(gpgme_key_t key){
    printf("Key ID: %s\n", key->subkeys->keyid);
    if(key->uids && key->uids->name)
        printf("Name: %s\n", key->uids->name);
    if(key->uids && key->uids->email)
        printf("E-Mail: %s\n\n", key->uids->email);

}

int listKeys(gpgme_ctx_t ctx, gpgme_error_t err, gpgme_key_t key, int onlyPrivateFlag){

    // start the keylist
    err = gpgme_op_keylist_start (ctx, NULL, onlyPrivateFlag);
    detectError(err);

    int nKeysFound=0;

    while (!(err = gpgme_op_keylist_next (ctx, &key))) { // loop through the keys in the keyring
        //printKeys(key);
        gpgme_key_unref (key); // release the key reference
        nKeysFound++;
    }

    return nKeysFound;

}

gpgme_genkey_result_t genKey(gpgme_ctx_t ctx, gpgme_error_t err, const char *parms){

    if(LOG) qDebug() << "Key pair generating...";

    gpgme_genkey_result_t result;

    err = gpgme_op_genkey (ctx, parms, NULL, NULL);
    detectError(err);

    result = gpgme_op_genkey_result (ctx);
    if (!result){
        fprintf (stderr, "%s:%d: gpgme_op_genkey_result returns NULL\n",
               __FILE__, __LINE__);
        exit (1);
    }
    if (result->fpr && strlen (result->fpr) != 40){
        fprintf (stderr, "%s:%d: generated key has unexpected fingerprint\n",
               __FILE__, __LINE__);
        exit (1);
    }
    if (!result->primary){
        fprintf (stderr, "%s:%d: primary key was not generated\n",
               __FILE__, __LINE__);
        exit (1);
    }
    if (!result->sub)
        {
        fprintf (stderr, "%s:%d: sub key was not generated\n",
               __FILE__, __LINE__);
        exit (1);
    }

    return result;
}

void exportKey(gpgme_ctx_t ctx, gpgme_key_t key, gpgme_error_t err, const char *outputFileName){

    //printKeys(key);
    gpgme_data_t out;
    FILE *outputFile;
    gpgme_key_t keys[2]={NULL, NULL};
    keys[0]=key;
    int ret;
    int BUF_SIZE = 512;
    char buf[BUF_SIZE + 1];

    err = gpgme_data_new(&out);
    detectError(err);

    err = gpgme_op_export_keys(ctx, keys, 0, out);
    detectError(err);

    fflush (NULL);
    // Open the output file
    outputFile = fopen (outputFileName, "w+");

    // Rewind the "out" data object
    ret = gpgme_data_seek (out, 0, SEEK_SET);
    // Error handling
    if (ret)
        detectError (gpgme_err_code_from_errno (errno));

    // Read the contents of "out" and place it into buf
    while ((ret = gpgme_data_read (out, buf, BUF_SIZE)) > 0) {
        // Write the contents of "buf" to "outputFile"
        fwrite (buf, ret, 1, outputFile);
    }

    // Error handling
    if (ret < 0)
        detectError (gpgme_err_code_from_errno (errno));

    // Close "outputFile"
    fclose(outputFile);

    gpgme_data_release (out);
}

gpgme_import_result_t importKey(gpgme_ctx_t ctx, gpgme_error_t err, const char *inFileName){

    if(LOG) printf("Import key from %s\n", inFileName);

    gpgme_data_t in;    //Data
    gpgme_import_result_t importResult; //Result

    //Assign gpgme data from key file.
    err = gpgme_data_new_from_file (&in, inFileName, 1);
    detectError(err);

    //Import key
    err = gpgme_op_import (ctx, in);
    detectError(err);

    //Get import result
    importResult = gpgme_op_import_result(ctx);

    gpgme_data_release (in); //Release data

    return importResult;
}

void encrypt(gpgme_ctx_t ctx, gpgme_error_t err, gpgme_key_t recv,
             const char *inputFileName, const char *outputFileName){

    if(LOG) qDebug() << "Encrypt "<<inputFileName<<"and placing the result into "<<outputFileName<<endl;

    gpgme_encrypt_result_t encryptResult;
    gpgme_data_t in, out;
    gpgme_key_t keys[2]={NULL, NULL};
    keys[0]=recv;

    FILE *outputFile;
    int BUF_SIZE = 512;
    char buf[BUF_SIZE + 1];
    int ret;

    err = gpgme_data_new_from_file (&in, inputFileName, 1);
    detectError(err);

    err = gpgme_data_new (&out);
    detectError(err);

    err = gpgme_op_encrypt (ctx, keys, GPGME_ENCRYPT_ALWAYS_TRUST, in, out);
    detectError(err);

    encryptResult = gpgme_op_encrypt_result (ctx);

    if (encryptResult->invalid_recipients) {
        detectError(err);
    }

    outputFile = fopen (outputFileName, "w+");

    // Rewind the "out" data object
    ret = gpgme_data_seek (out, 0, SEEK_SET);
    // Error handling
    if (ret)
        detectError(gpgme_err_code_from_errno (errno));

    // Read the contents of "out" and place it into buf
    while ((ret = gpgme_data_read (out, buf, BUF_SIZE)) > 0) {
        // Write the contents of "buf" to "outputFile"
        fwrite (buf, ret, 1, outputFile);
    }

    // Error handling
    if (ret < 0)
        detectError(gpgme_err_code_from_errno (errno));

    // Close "outputFile"
    fclose(outputFile);

    // Release the "in" data object
    gpgme_data_release (in);
    // Release the "out" data object
    gpgme_data_release (out);

}

void encryptSign(gpgme_ctx_t ctx, gpgme_error_t err, gpgme_key_t recpKey,
                 const char *inputFileName,const char *outputFileName){    //Not finished yet.

    gpgme_encrypt_result_t encryptResult;
    gpgme_key_t keys[2]={NULL, NULL};
    keys[0]=recpKey;
    gpgme_data_t in, out;
    FILE *outputFile;
    int ret;
    int BUF_SIZE = 512;
    char buf[BUF_SIZE + 1];

    err = gpgme_data_new_from_file (&in, inputFileName, 1);
    detectError(err);

    err = gpgme_data_new (&out);
    detectError(err);

    err=gpgme_op_encrypt_sign(ctx, keys, GPGME_ENCRYPT_ALWAYS_TRUST, in, out);
    detectError(err);

    encryptResult = gpgme_op_encrypt_result (ctx);
    if (encryptResult->invalid_recipients) {
        detectError(err);
    }

    outputFile = fopen (outputFileName, "w+");

    // Rewind the "out" data object
    ret = gpgme_data_seek (out, 0, SEEK_SET);
    // Error handling
    if (ret)
        detectError(gpgme_err_code_from_errno (errno));

    // Read the contents of "out" and place it into buf
    while ((ret = gpgme_data_read (out, buf, BUF_SIZE)) > 0) {
        // Write the contents of "buf" to "outputFile"
        fwrite (buf, ret, 1, outputFile);
    }

    // Error handling
    if (ret < 0)
        detectError(gpgme_err_code_from_errno (errno));

    // Close "outputFile"
    fclose(outputFile);

    // Release the "in" data object
    gpgme_data_release (in);
    // Release the "out" data object
    gpgme_data_release (out);


}

void decrypt(gpgme_ctx_t ctx, gpgme_error_t err, const char *inputFileName,
             const char *outputFileName){

    gpgme_decrypt_result_t decryptResult;
    gpgme_data_t in, out;
    FILE *outputFile;
    int BUF_SIZE = 512;
    char buf[BUF_SIZE + 1];
    int ret;

    err=gpgme_data_new_from_file(&in, inputFileName, 1);
    detectError(err);

    err=gpgme_data_new(&out);
    detectError(err);

    gpgme_op_decrypt(ctx, in, out);
    detectError(err);

    decryptResult = gpgme_op_decrypt_result(ctx);
    if(decryptResult->wrong_key_usage){
        detectError(err);
    }


    outputFile = fopen (outputFileName, "w+");

    // Rewind the "out" data object
    ret = gpgme_data_seek (out, 0, SEEK_SET);
    // Error handling
    if (ret)
        detectError(gpgme_err_code_from_errno (errno));

    // Read the contents of "out" and place it into buf
    while ((ret = gpgme_data_read (out, buf, BUF_SIZE)) > 0) {
        // Write the contents of "buf" to "outputFile"
        fwrite (buf, ret, 1, outputFile);
    }

    // Error handling
    if (ret < 0)
        detectError(gpgme_err_code_from_errno (errno));

    // Close "outputFile"
    fclose(outputFile);

    // Release the "in" data object
    gpgme_data_release (in);
    // Release the "out" data object
    gpgme_data_release (out);


}

QString decryptVerify(gpgme_ctx_t ctx, gpgme_error_t err, const char *inputFileName,
             const char *outputFileName){

    gpgme_decrypt_result_t decryptResult;
    gpgme_verify_result_t verifyResult;
    gpgme_data_t in, out;
    FILE *outputFile;
    int BUF_SIZE = 512;
    char buf[BUF_SIZE + 1];
    int ret;

    err=gpgme_data_new_from_file(&in, inputFileName, 1);
    detectError(err);

    err=gpgme_data_new(&out);
    detectError(err);

    gpgme_op_decrypt_verify(ctx, in, out);
    detectError(err);

    decryptResult = gpgme_op_decrypt_result(ctx);
    if(decryptResult->wrong_key_usage){
        detectError(err);
    }
    verifyResult = gpgme_op_verify_result(ctx);

    outputFile = fopen (outputFileName, "w+");

    // Rewind the "out" data object
    ret = gpgme_data_seek (out, 0, SEEK_SET);
    // Error handling
    if (ret)
        detectError(gpgme_err_code_from_errno (errno));

    // Read the contents of "out" and place it into buf
    while ((ret = gpgme_data_read (out, buf, BUF_SIZE)) > 0) {
        // Write the contents of "buf" to "outputFile"
        fwrite (buf, ret, 1, outputFile);
    }

    // Error handling
    if (ret < 0)
        detectError(gpgme_err_code_from_errno (errno));

    // Close "outputFile"
    fclose(outputFile);

    // Release the "in" data object
    gpgme_data_release (in);
    // Release the "out" data object
    gpgme_data_release (out);

    gpgme_signature_t sig=verifyResult->signatures;
    QString result;
    QString fpr=QString(sig->fpr);

    if ((sig->summary & GPGME_SIGSUM_RED)){
       result="0"+fpr;
    }
    else{
       result="1"+fpr;

    }

    return result;

}

/*
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    gpgme_ctx_t ctx;  // the context
    gpgme_error_t err; // errors
    gpgme_key_t key= nullptr; // the key
    gpgme_key_t senderKey;
    gpgme_key_t recipientKey;


    //Begin setup of GPGME
    gpgme_check_version (NULL);
    setlocale (LC_ALL, "");
    gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
//#ifndef HAVE_W32_SYSTEM
//    gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
//#endif
    //err = gpgme_engine_check_version (GPGME_PROTOCOL_OpenPGP);
    //detectError(err);
    //End setup of GPGME

    // Create the GPGME Context
    err = gpgme_new (&ctx);
    // Error handling
    detectError(err);

    if(LOG) qDebug() << "Create the GPGME Context SUCCESS";

    // Set the context to textmode
    gpgme_set_textmode (ctx, 1);
    // Enable ASCII armor on the context
    gpgme_set_armor (ctx, 1);


    if(LOG) qDebug() << "1---------Listing Private Keys----------1";
    int nPrivateKeys = listKeys(ctx, err, key, 1);
    if(LOG) qDebug() << "\n---------FOUND "<< nPrivateKeys << " Private Keys----------\n";

    if(LOG) qDebug() << "2---------Listing Public Keys-----------2";
    int nPublicKeys = listKeys(ctx, err, key, 0);
    if(LOG) qDebug() << "\n---------FOUND "<< nPublicKeys << " Public Keys----------\n";



    if(LOG) qDebug() << "3---------A New Key Generation----------3";
    const char *parms = "<GnupgKeyParms format=\"internal\">\n"
        "Key-Type: RSA\n"
        "Key-Length: 4096\n"
        "Subkey-Type: RSA\n"
        "Subkey-Length: 4096\n"
        "Name-Real: E2 Eeim\n"
        "Name-Comment: (Generated by E2EEIM Chat, Passphrase:1234567890)\n"
        "Name-Email: e2eeim@e2eeim.crypto\n"
        "Expire-Date: 1d\n"
        "Passphrase: 1234567890\n"
        "</GnupgKeyParms>\n";

    gpgme_genkey_result_t GenKeyresult;
    GenKeyresult = genKey(ctx, err, parms);
    if(LOG) qDebug() << "--------Finished Key Generation----------";
    err = gpgme_get_key (ctx,GenKeyresult->fpr,&key,1);
    detectError(err);
    printKeys(key);



    if(LOG) qDebug() << "4----------Export Public Keys-----------4";
    const char *outFile="E2EEIM_Public_Key.key";
    exportKey(ctx, key, err, outFile);
    if(LOG) qDebug() << "'E2EEIM_Public_Key.key' Saved in current directory\n\n";

    if(LOG) qDebug() << "5----------Import Public Keys-----------5";
    const char *inFile="E2EEIM_Public_Key.key";
    gpgme_import_result_t importResult;
    importResult = importKey(ctx, err, inFile);

    if(LOG) qDebug() <<"considered: " << importResult->considered;
    if(LOG) qDebug() <<"imported: " << importResult->imported;
    if(LOG) qDebug() <<"secret_imported: " << importResult->secret_imported;
    if(LOG) qDebug() <<"new_signatures: " << importResult->new_signatures;
    if(LOG) qDebug() <<"new_revocation: " << importResult->new_revocations;
    if(LOG) qDebug() << "new_sub_keys: " << importResult->new_sub_keys;
    if(LOG) qDebug() << "unchanged: " << importResult->unchanged;
    if(LOG) qDebug() << "imported_rsa: " << importResult->imported_rsa;

    if(LOG) qDebug() << "\n\n";


    senderKey = key;


    if(LOG) qDebug() << "6---------A New Key Generation----------6";
    parms = "<GnupgKeyParms format=\"internal\">\n"
        "Key-Type: RSA\n"
        "Key-Length: 4096\n"
        "Subkey-Type: RSA\n"
        "Subkey-Length: 4096\n"
        "Name-Real: Recv Eeim\n"
        "Name-Comment: (Generated by E2EEIM Chat, Passphrase:abcdefgh)\n"
        "Name-Email: recv@e2eeim.crypto\n"
        "Expire-Date: 1d\n"
        "Passphrase: abcdefgh\n"
        "</GnupgKeyParms>\n";

    GenKeyresult = genKey(ctx, err, parms);
    if(LOG) qDebug() << "--------Finished Key Generation----------";
    err = gpgme_get_key (ctx,GenKeyresult->fpr,&key,1);
    detectError(err);
    printKeys(key);

    recipientKey = key;

    if(LOG) qDebug() << "7---------Symmetric Encryption----------7";
    const char *inputEncrypt="Makefile";
    const char *outputEncrypt="Makefile.SymEncrypted";
    encrypt(ctx, err, recipientKey, inputEncrypt, outputEncrypt);

    if(LOG) qDebug() << "Finished encryption, output saved into" << outputEncrypt << endl;


    if(LOG) qDebug() << "8-------- Symmetric Decryption-----------8";
    const char *inputDecrypt="Makefile.SymEncrypted";
    const char *outputDecrypt="Makefile.SymDecrypted";
    decrypt(ctx, err, inputDecrypt, outputDecrypt);
    if(LOG) qDebug() << "Finished decryption, output saved into" << outputDecrypt << endl;

    // //////////////////////////////////////////////////////

    if(LOG) qDebug() << "9--------Asymmetric Encryption-----------9";
    const char *outputEncrypt2="Makefile.AsymEncrypted";

    gpgme_signers_add(ctx, senderKey);

    encryptSign(ctx, err, recipientKey, inputEncrypt, outputEncrypt2);
    if(LOG) qDebug() << "Finished encryption, output saved into" << outputEncrypt2 << endl;


    if(LOG) qDebug() << "10--------Asymmetric Decryption----------10";
    const char *inputDecrypt2="Makefile.AsymEncrypted";
    const char *outputDecrypt2="Makefile.AsymDecrypted";
    gpgme_verify_result_t verifyResult;
    verifyResult = decryptVerify(ctx, err, inputDecrypt2, outputDecrypt2);
    if(LOG) qDebug() << "Finished decryption, output saved into" << outputDecrypt2 << endl;


    if(LOG) qDebug() << "11--------------Delete Keys--------------11";

    printKeys(senderKey);
    err = gpgme_op_delete (ctx,senderKey,1);
    //detectError(err);
    //if(LOG) qDebug() << "DELETED!!\n";

    printKeys(recipientKey);
    err = gpgme_op_delete (ctx,recipientKey,1);
    //detectError(err);
    //if(LOG) qDebug() << "DELETED!!\n";

    if(LOG) qDebug() << "\n\nALL DONE\n\n";

    if(LOG) qDebug() << "At runtime, the program generated 5 files as follow";
    if(LOG) qDebug() << "E2EEIM_Public_Key.key";
    if(LOG) qDebug() << "Makefile.SymEncrypted";
    if(LOG) qDebug() << "Makefile.SymDncrypted";
    if(LOG) qDebug() << "Makefile.AsymEncrypted";
    if(LOG) qDebug() << "Makefile.AsymDncrypted";
    if(LOG) qDebug() << "\nYou can delete it all by 'make distclean'";
    if(LOG) qDebug() << "(If 'Makefile' generated by 'qmake')";


    gpgme_release(ctx); // release the context, all done
    return 0;

    return a.exec();
}
*/

#endif // ENCRYPTION_H
