@echo off

rem Set environment variables for dataStr, toIP, port, sslPrivateKeyPath, sslLocalCertificatePath, and sslCaPath
@echo dataStr=%1
@echo toIP=%2
@echo port=%3
@echo sslPrivateKeyPath=%4
@echo sslLocalCertificatePath=%5
@echo sslCaPath=%6

rem Create an empty "session.pem" file
type nul > session.pem

rem Run your long OpenSSL command
echo %dataStr% | openssl s_client -dtls1_2 -connect %toIP%:%port% -sess_out session.pem -key %sslPrivateKeyPath% -cert %sslLocalCertificatePath% -CAfile %sslCaPath% -verify 2 -cipher AES256-GCM-SHA384
