#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <regex>

using namespace geode::prelude;

std::string customTitleLogo = "Geometry Dash";
std::string cachedEWDString = "Ruminative Dash";
int errorCode = 0;

bool setupTitleLogoReplacement(CCSprite* titleLogo)
{
    auto limitLabelWidth = [](CCLabelBMFont* label, const float width, const float defaultScale, const float minScale) {
        if (!label) return false;

        const float originalWidth = label->getContentSize().width;
        float currentScale = label->getScale();
        float requested = (defaultScale > 0.0f) ? defaultScale : currentScale;

        log::debug("[limitLabelWidth] originalWidth: {}, currentScale: {}, requested: {}",
            originalWidth, currentScale, requested);

        if (originalWidth <= 0.0f || width <= 0.0f) {
            label->setScale(requested);
            return true;
        }

        const float maxAllowed = width / originalWidth;
        float finalScale = requested;

        if (requested > 1.0f) {
            if (originalWidth * requested <= width) finalScale = requested;
            else finalScale = std::min(requested, maxAllowed);
        } else {
            if (originalWidth * requested <= width) finalScale = requested;
            else finalScale = maxAllowed;
        }

        if (minScale > 0.0f && finalScale < minScale) finalScale = minScale;

        label->setScale(finalScale);
        return true;
    };
    if (!titleLogo) return false;

    titleLogo->setOpacity(0);

    std::string temp = customTitleLogo;
    if (Mod::get()->getSettingValue<bool>("force-uppercase")) std::ranges::transform(temp, temp.begin(), [](const unsigned char c){ return std::toupper(c); });

    CCLabelBMFont* newTitleLogo = CCLabelBMFont::create(temp.c_str(), "merged_output.fnt"_spr);
    CCLabelBMFont* newTitleLogoUnderlay = CCLabelBMFont::create(temp.c_str(), "underlay_bigsheet.fnt"_spr);
    if (!newTitleLogo || !newTitleLogoUnderlay) return false;

    const float predeterminedWidth = CCDirector::sharedDirector()->getWinSize().width * 0.75f;

    limitLabelWidth(newTitleLogo, predeterminedWidth, 1.25f, 0.25f);

    newTitleLogo->setID("custom-main-title"_spr);
    newTitleLogo->setZOrder(1);
    titleLogo->addChild(newTitleLogo);

    limitLabelWidth(newTitleLogoUnderlay, predeterminedWidth, 1.25f, 0.25f);

    newTitleLogoUnderlay->setID("custom-main-title-underlay"_spr);
    newTitleLogoUnderlay->setZOrder(0);
    titleLogo->addChild(newTitleLogoUnderlay);

    titleLogo->updateLayout();

    newTitleLogo->setPosition(titleLogo->getContentSize() / 2.f);
    newTitleLogo->setPosition(newTitleLogo->getPositionX() + 6.f, newTitleLogo->getPositionY() - 4.f);
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
        // darkmode "compatibility, thank you bitz"
        if (Result<> res = self.setHookPriorityAfterPost("LoadingLayer::init", "bitz.darkmode_v4"); !res || res.isErr()) log::error("[TLTLoadingLayer::onModify] Failed to set hook priority: {}", res.unwrapErr());
        // else log::debug("[TLTLoadingLayer::onModify] Successfully set hook priority after post for LoadingLayer::init");
    }

    $override bool init(const bool fromRefresh)
    {
        errorCode = 0;
        std::string rawResponse = "ooh shiny";
        if (!fromRefresh)
        {
            // fetch the last string if the web req somehow didn't make it
            const auto savedCache = Mod::get()->getSavedValue<std::string>("cached-ewd-string");
            cachedEWDString = savedCache;
            if (savedCache.empty()) {
                errorCode += 1;
                cachedEWDString = "Ruminative Dash";
            }

            if (Mod::get()->getSettingValue<bool>("aggressive-prefetch"))
            {
                auto req = web::WebRequest().timeout(std::chrono::seconds(3));
                web::WebTask etask = req.get("https://raw.githubusercontent.com/AnhNguyenlost13/every-word-dash-api/refs/heads/master/badeline.txt");
                while (etask.isPending()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (rawResponse = etask.getFinishedValue()->string().unwrapOr("."); rawResponse == ".") errorCode += 1;
            } else
            {
                // listener bind
                m_fields->m_listener.bind([&rawResponse] (web::WebTask::Event* event) mutable {
                    if (const web::WebResponse* response = event->getValue()) rawResponse = response->string().unwrapOr(".");
                    else if (event->isCancelled()) errorCode += 2;
                });

                // now we fetch (passively, don't hog the game)
                m_fields->m_listener.setFilter(web::WebRequest().get("https://raw.githubusercontent.com/AnhNguyenlost13/every-word-dash-api/refs/heads/master/badeline.txt"));
                if (rawResponse == "." || rawResponse.empty()) errorCode += 4;
                else cachedEWDString = rawResponse;
            }

            if (rawResponse != cachedEWDString) Mod::get()->setSavedValue("cached-ewd-string", rawResponse);

            std::ranges::transform(cachedEWDString, cachedEWDString.begin(), [](const unsigned char c){ return std::toupper(c); });

            CCFileUtils::sharedFileUtils()->addSearchPath((Mod::get()->getTempDir() / "resources").string().c_str());

            CCTextureCache* textureCache = CCTextureCache::sharedTextureCache();
            (void) textureCache->addImage("merged_atlas.png"_spr, false);
            (void) textureCache->addImage("underlay_atlas.png"_spr, false);

            CCLabelBMFont::create("trans rights are human rights", "merged_output.fnt"_spr);
            CCLabelBMFont::create("if you're curious why I have to create these labels, uhhh idk either lol rob does it so do I", "underlay_bigsheet.fnt"_spr);
        }

        if (Mod::get()->getSettingValue<bool>("every-word-dash-integration")) customTitleLogo = cachedEWDString;
        else customTitleLogo = Mod::get()->getSettingValue<std::string>("custom-title-logo");

        if (!LoadingLayer::init(fromRefresh)) return false;

        if (const auto titleLogo = typeinfo_cast<CCSprite*>(getChildByIDRecursive("gd-logo")); !titleLogo || !setupTitleLogoReplacement(titleLogo)) return true;
    }
};

class $modify(TLTMenuLayer, MenuLayer) {
    $override bool init() override {
        if (!MenuLayer::init()) return false;

        const auto titleLogo = typeinfo_cast<CCSprite*>(getChildByIDRecursive("main-title"));
        if (!titleLogo) return true;

        if (!Mod::get()->getSettingValue<bool>("every-word-dash-integration")) customTitleLogo = Mod::get()->getSettingValue<std::string>("custom-title-logo");
        else {
            if (errorCode > 1)
                Notification::create(
                    "Failed to fetch/parse EWD string (errcode " + std::to_string(errorCode) + ")",
                    NotificationIcon::Error,
                    1.f
                )->show();
            else customTitleLogo = cachedEWDString;
        }

        if (!setupTitleLogoReplacement(titleLogo)) return true;
    }
};
