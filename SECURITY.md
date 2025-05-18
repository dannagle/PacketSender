# Security Policy

## Supported Versions

Packet Sender will always support free security fixes for the latest version. If the problem is determined to be a medium or high vulnerability, either a fix or a mitigation technique will be listed here within 60 days. 


| Version | Supported          |
| ------- | ------------------ |
| 8.8.9   | :white_check_mark: |
| 8.8.2   | :x: |
| 8.7.1   | :white_check_mark: |
| older   | :x: |


## Reporting a Vulnerability

- Use the standard [bug reporting process](/dannagle/PacketSender/issues/new/choose). 
- Use https://packetsender.com/contact if you do not wish to use a GitHub account.


## Release signing

For Windows, every release of Packet Sender from 2022 onward has been code-signed "NagleCode, LLC" with Extended Validation from a FIPS compliant hardware token. This is the strongest code signing certification available on the market. Modified installers and executables will not have the correct digital signature if you right-click view properties. 

For Mac, every release of Packet Sender from 2020 onward has been code-signed and notarized using Apple provided certificates and web hooks. Most Macs are configured to not allow launching the installer or app if the digital signature has been altered.

For Linux, there are no digital signatures. For those concerned about this, only download the .AppImage directly from the GitHub release page, or you may compile by source. 


## List of known vulnerabilities with mitigations

- The shipped OpenSSL libraries are old (Windows only).
The mitigation is to delete libssl-3-x64.dll and libcrypto-3-x64.dll. Packet Sender will then use the built-in Windows Secure Channel for SSL connections. You will lose some capabilities provided by OpenSSL, but that is probably OK for most. 

- Packet Sender ships with untrusted snake oil certs (Windows only). 
This is by design so Packet Sender can provide SSL test servers out-of-the-box. You can override with your own certs in \AppData\Local\PacketSender key.pem and cert.pem. Also, Packet Sender now ships with default off SSL servers.

- Packet Sender ignores SSL errors.
This is by design so Packet Sender can be used to troubleshoot these kinds of connections. You can go to Settings and turn off "Ignore SSL Errors" to make Packet Sender behave as a normal client. 


