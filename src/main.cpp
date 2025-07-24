#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>

using namespace geode::prelude;

std::string customTitleLogo = "Geometry Dash";
std::string cachedEWDString = "";
// $execute{
//     EventListener<web::WebTask> listener;
//
//     listener.bind([] (web::WebTask::Event& event) {
//         if (const web::WebResponse* response = event.getValue())
//         {
//             gd::string res = response->string().unwrapOr("Geometry Dash");
//             std::transform(res.begin(), res.end(), res.begin(), [](const unsigned char c){ return std::toupper(c); });
//         } else if (event.isCancelled()) log::warn("req cancelled");
//     });
//
//     auto request = web::WebRequest();
//     if (Mod::get()->getSettingValue<bool>("every-word-dash-integration")) listener.setFilter(request.get("https://raw.githubusercontent.com/AnhNguyenlost13/super-duper-garbanzo/refs/heads/main/badeline.txt"));
// }

bool setupTitleLogoReplacement(CCSprite* titleLogo, bool loadingLayer)
{
    titleLogo->setOpacity(0);

    std::string temp = customTitleLogo;
    if (Mod::get()->getSettingValue<bool>("force-uppercase")) std::ranges::transform(temp, temp.begin(), [](const unsigned char c){ return std::toupper(c); });

    CCLabelBMFont* newTitleLogo = CCLabelBMFont::create(temp.c_str(), "merged_output.fnt"_spr);
    CCLabelBMFont* newTitleLogoUnderlay = CCLabelBMFont::create(temp.c_str(), "underlay_bigsheet.fnt"_spr);
    if (!newTitleLogo || !newTitleLogoUnderlay) {
        log::error("Something went very wrong!");
        return false;
    }

    newTitleLogo->limitLabelWidth(400.f, 1.25f, 0.25f);
    newTitleLogo->setID("custom-main-title"_spr);
    newTitleLogo->setZOrder(1);
    titleLogo->addChild(newTitleLogo);

    newTitleLogoUnderlay->limitLabelWidth(400.f, 1.25f, 0.25f);
    newTitleLogoUnderlay->setID("custom-main-title-underlay"_spr);
    newTitleLogoUnderlay->setZOrder(0);
    titleLogo->addChild(newTitleLogoUnderlay);
    titleLogo->updateLayout();

    if (loadingLayer) titleLogo->setScale(1.2f);
    const auto center = titleLogo->getContentSize() / 2.f;
    newTitleLogo->setPosition(center);
    newTitleLogoUnderlay->setPosition(newTitleLogo->getPosition());

    return true;
}

class $modify(TLTLoadingLayer, LoadingLayer)
{
    struct Fields
    {
        EventListener<web::WebTask> m_listener;
    };

    static void onModify(auto& self)
    {
        if (Result<> res = self.setHookPriorityPre("LoadingLayer::init", Priority::Late); !res || res.isErr()) log::error("Failed to set hook priority: {}", res.unwrapErr());
    }

    $override bool init(const bool fromRefresh)
    {
        // m_fields->m_listener.bind([this] (web::WebTask::Event* event)
        // {
        //     if (const web::WebResponse* response = event->getValue())
        //     {
        //         gd::string res = response->string().unwrapOr("Geometry Dash");
        //         std::ranges::transform(res, res.begin(), [](const unsigned char c){ return std::toupper(c); });
        //         // customTitleLogo = res;
        //         cachedEWDString = res;
        //         // should be uppercase???
        //         log::info("EWD string: {}", res);
        //     }
        //     else if (event->isCancelled()) log::warn("req cancelled");
        // });

        // Initial setup - only on first load
        if (!fromRefresh)
        {
            web::WebTask task = web::WebRequest().get("https://raw.githubusercontent.com/AnhNguyenlost13/super-duper-garbanzo/refs/heads/main/badeline.txt");
            while (task.isPending()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cachedEWDString = task.getFinishedValue()->string().unwrapOr("Geometry Dash"); // There you go, if it fails it just falls back to "Geometry Dash"
            // and yes caching is there until the next game restart.
            std::ranges::transform(cachedEWDString, cachedEWDString.begin(), [](const unsigned char c){ return std::toupper(c); });

            CCFileUtils::sharedFileUtils()->addSearchPath((Mod::get()->getTempDir() / "resources").string().c_str());
            CCTextureCache* textureCache = CCTextureCache::sharedTextureCache();
            (void) textureCache->addImage("merged_atlas.png", false);
            (void) textureCache->addImage("underlay_atlas.png", false);
            CCLabelBMFont::create("trans rights are human rights", "merged_output.fnt"_spr);
            CCLabelBMFont::create("i should be more careful with bets next time", "underlay_bigsheet.fnt"_spr);
        }

        if (Mod::get()->getSettingValue<bool>("every-word-dash-integration")) customTitleLogo = cachedEWDString;
        else customTitleLogo = Mod::get()->getSettingValue<std::string>("custom-title-logo");

        if (!LoadingLayer::init(fromRefresh)) return false;

        return setupTitleLogoReplacement(typeinfo_cast<CCSprite*>(getChildByIDRecursive("gd-logo")), true);
    }
};

class $modify(TLTMenuLayer, MenuLayer) {
    $override bool init() override {
        bool result = MenuLayer::init();

        const auto titleLogo = typeinfo_cast<CCSprite*>(getChildByIDRecursive("main-title"));
        if (!titleLogo) {
            log::error("whoever removed the node with ID 'main-title' from the MenuLayer, you are a menace to society!");
            return result;
        }

        if (!Mod::get()->getSettingValue<bool>("every-word-dash-integration")) customTitleLogo = Mod::get()->getSettingValue<std::string>("custom-title-logo");
        else {
            if (cachedEWDString.empty())
            {
                customTitleLogo = "GEOMETRY DASH";
                FLAlertLayer::create(
                    "Failed to get EWD string from cache",
                    "oops sorry, it's me fumbling as always",
                    "OK"
                )->show();
            } else customTitleLogo = cachedEWDString;
        }

        return setupTitleLogoReplacement(titleLogo, false);
    }
};