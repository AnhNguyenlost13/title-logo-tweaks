#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <regex>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/web.hpp>

#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>

using namespace geode::prelude;

std::string customTitleLogo = "Geometry Dash";
std::string cachedEWDString = "Default String";
std::atomic<int> errorCode{0};
static std::mutex cachedMutex;

bool setupTitleLogoReplacement(CCSprite* titleLogo)
{
    auto limitLabelWidth = [](CCLabelBMFont* label, const float width, const float defaultScale, const float minScale) {
        if (!label) return false;

        const float originalWidth = label->getContentSize().width;
        const float currentScale = label->getScale();
        const float requested = (defaultScale > 0.0f) ? defaultScale : currentScale;

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

    std::string temp;
    {
        std::scoped_lock lock(cachedMutex);
        temp = customTitleLogo;
    }

    if (Mod::get()->getSettingValue<bool>("force-uppercase")) std::ranges::transform(temp.begin(), temp.end(), temp.begin(), [](const unsigned char c){ return static_cast<char>(std::toupper(c)); });

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
        if (const Result<> res = self.setHookPriorityAfterPre("LoadingLayer::init", "weebify.high-graphics-android"); !res || res.isErr()) log::error("could not set prio: {}", res.unwrapErr());
    }

    $override bool init(const bool fromRefresh)
    {
        errorCode.store(0);

        auto rawResponsePtr = std::make_shared<std::string>(".");

        if (!fromRefresh)
        {
            const auto savedCache = Mod::get()->getSavedValue<std::string>("cached-ewd-string");
            {
                std::scoped_lock lock(cachedMutex);
                if (!savedCache.empty()) cachedEWDString = savedCache;
            }

            if (savedCache.empty()) errorCode.fetch_add(1);
            else *rawResponsePtr = savedCache;

            if (Mod::get()->getSettingValue<bool>("aggressive-prefetch"))
            {
                auto req = web::WebRequest().timeout(std::chrono::seconds(3));
                web::WebTask etask = req.get("https://raw.githubusercontent.com/AnhNguyenlost13/every-word-dash-api/refs/heads/master/badeline.txt");

                constexpr auto maxWait = std::chrono::milliseconds(3000);
                auto waited = std::chrono::milliseconds(0);
                constexpr auto step = std::chrono::milliseconds(1);
                while (etask.isPending() && waited < maxWait) {
                    std::this_thread::sleep_for(step);
                    waited += step;
                }

                if (const auto finished = etask.getFinishedValue(); finished && finished->ok()) {
                        *rawResponsePtr = finished->string().unwrapOr(".");
                        if ((*rawResponsePtr) == ".")errorCode.fetch_add(1);
                        else errorCode.fetch_add(1);
                } else errorCode.fetch_add(1);
            }
            else
            {
                m_fields->m_listener.bind([rawResponsePtr] (web::WebTask::Event* event) mutable {
                    if (const web::WebResponse* response = event->getValue()) {
                        if (response->ok()) {
                            *rawResponsePtr = response->string().unwrapOr(".");

                            if (!rawResponsePtr->empty() && *rawResponsePtr != ".") {
                                std::scoped_lock lock(cachedMutex);
                                cachedEWDString = *rawResponsePtr;
                                Mod::get()->setSavedValue("cached-ewd-string", cachedEWDString);
                            } else errorCode.fetch_add(1);
                        } else errorCode.fetch_add(1);
                    }
                    else if (event->isCancelled()) errorCode.fetch_add(2);
                });

                m_fields->m_listener.setFilter(web::WebRequest().get("https://raw.githubusercontent.com/AnhNguyenlost13/every-word-dash-api/refs/heads/master/badeline.txt"));

                if ((*rawResponsePtr) == "." || rawResponsePtr->empty()) errorCode.fetch_add(4);
                else {
                    std::scoped_lock lock(cachedMutex);
                    cachedEWDString = *rawResponsePtr;
                }
            }

            {
                std::scoped_lock lock(cachedMutex);
                if ((*rawResponsePtr) != cachedEWDString && !rawResponsePtr->empty() && (*rawResponsePtr) != ".") {
                    Mod::get()->setSavedValue("cached-ewd-string", *rawResponsePtr);
                    cachedEWDString = *rawResponsePtr;
                }
            }

            {
                std::string upper;
                {
                    std::scoped_lock lock(cachedMutex);
                    upper = cachedEWDString;
                }
                std::ranges::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
                {
                    std::scoped_lock lock(cachedMutex);
                    cachedEWDString = std::move(upper);
                }
            }

            CCFileUtils::sharedFileUtils()->addSearchPath((Mod::get()->getTempDir() / "resources").string().c_str());

            CCTextureCache* textureCache = CCTextureCache::sharedTextureCache();
            auto tex1 = textureCache->addImage("merged_atlas.png"_spr, false);
            auto tex2 = textureCache->addImage("underlay_atlas.png"_spr, false);

            auto labelA = CCLabelBMFont::create("trans rights are human rights", "merged_output.fnt"_spr);
            auto labelB = CCLabelBMFont::create("if you're curious why I have to create these labels, uhhh idk either lol rob does it so do I", "underlay_bigsheet.fnt"_spr);
        }

        if (Mod::get()->getSettingValue<bool>("every-word-dash-integration")) {
            std::scoped_lock lock(cachedMutex);
            customTitleLogo = cachedEWDString;
        }
        else customTitleLogo = Mod::get()->getSettingValue<std::string>("custom-title-logo");

        if (!LoadingLayer::init(fromRefresh)) return false;

        const auto titleLogo = typeinfo_cast<CCSprite*>(getChildByIDRecursive("gd-logo"));
        if (!titleLogo) return true;
        setupTitleLogoReplacement(titleLogo);
        return true;
    }
};

class $modify(TLTMenuLayer, MenuLayer) {
    $override bool init() override {
        if (!MenuLayer::init()) return false;

        const auto titleLogo = typeinfo_cast<CCSprite*>(getChildByIDRecursive("main-title"));
        if (!titleLogo) return true;

        if (!Mod::get()->getSettingValue<bool>("every-word-dash-integration")) customTitleLogo = Mod::get()->getSettingValue<std::string>("custom-title-logo");
        else
        {
            if (errorCode.load() > 1) {
                if (errorCode.load() == 5) {
                    Notification::create(
                    "Restart the game for the latest word, sorry!",
                    NotificationIcon::Info,
                    1.f
                    )->show();
                } else {
                    Notification::create(
                    fmt::format("Failed to fetch/parse EWD string (errcode {})", errorCode.load()),
                    NotificationIcon::Error,
                    1.f
                    )->show();
                }
            }
            else {
                std::scoped_lock lock(cachedMutex);
                customTitleLogo = cachedEWDString;
            }
        }

        setupTitleLogoReplacement(titleLogo);
        return true;
    }
};
