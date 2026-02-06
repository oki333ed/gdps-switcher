#include "ServerInfoManager.hpp"
#include "utils/GDPSMain.hpp"

using namespace geode::prelude;

ServerInfoManager *ServerInfoManager::m_instance = nullptr;

void ServerInfoManager::fetch(GDPSTypes::Server& server) {
    if (server.infoLoaded) return;
    server.infoLoaded = true;
    if (server.id < 0) return;
    // idk weird stuff
    int id = server.id;
    std::string url = server.url;
    
    auto req = web::WebRequest();
    std::string endpoint = server.url;
    if (!endpoint.empty() && endpoint.back() != '/')
        endpoint += '/';
    endpoint += "switcher/getInfo.php";

    m_listeners[server.id].spawn(
        req.get(server.url),
        [this, &server] (web::WebResponse value) {
            auto json = value.json();
            if (json.isErr()) {
                // This little guy right here explodes GD if run.
                // I don't know why or how but the json.err() into the format args causes a bad alloc
                //log::warn("Failed to parse info for {}: {}", server.url, json.err());
                log::warn("Failed to parse info for {}", server.url);
                return;
            }
            auto info = json.unwrap();
            server.motd = info["motd"].asString().unwrapOr("No MOTD found.");
            server.icon = info["icon"].asString().unwrapOr("");
            // serverData.modPolicy = info["mods"]["policy"].asString().unwrapOr(serverData.modPolicy);
            // serverData.dependencies = info["mods"]["dependencies"].as<std::map<std::string, std::string>>().unwrapOr(serverData.dependencies);
            // serverData.modList = info["mods"]["modList"].as<std::vector<std::string>>().unwrapOr(serverData.modList);
            auto ev = LoadDataEventData(server);
            LoadDataEvent().send(&ev);
        }
    );
}

GDPSTypes::Server& LoadDataEventData::getServer() const {
    return m_server;
}

ServerInfoManager *ServerInfoManager::get() {
    if (!m_instance) m_instance = new ServerInfoManager;
    return m_instance;
}