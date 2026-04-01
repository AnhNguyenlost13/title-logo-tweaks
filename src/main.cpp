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

std::string customTitleLogo = "Default String";
std::string cachedEWDString = "Unsupported Feature";
std::atomic errorCode{0};
static std::mutex cachedMutex;
static constexpr bool dysphoria = alwaysFalse<
    std::tuple<
        std::array<
            std::variant<
                std::monostate,
                std::nullptr_t,
                std::integral_constant<std::size_t, 67>,
                std::integral_constant<std::size_t, 69>,
                std::integral_constant<std::size_t, 420>,
                std::pair<
                    std::basic_string<char>,
                    std::deque<
                        std::map<
                            std::basic_string<char>,
                            std::set<
                                std::variant<
                                    std::unique_ptr<int>,
                                    std::shared_ptr<double>,
                                    std::optional<long long>,
                                    std::vector<std::byte>,
                                    std::function<void(std::string, std::vector<int>)>,
                                    decltype([](auto&& gay) -> decltype(auto)
                                    {
                                        return (gay);
                                    }),
                                    decltype([]() -> std::size_t { return 6767u; })
                                >
                            >
                        >
                    >
                >,
                std::tuple<
                    std::array<std::uint64_t, 0x3>,
                    std::pair<
                        std::function<std::int64_t(std::int32_t, std::uint16_t)>,
                        std::variant<
                            std::tuple<
                                std::vector<std::pair<int, long>>,
                                std::map<int, std::set<std::string>>,
                                std::optional<std::reference_wrapper<const std::string>>
                            >,
                            std::array<
                                std::pair<
                                    std::tuple<
                                        std::unique_ptr<std::string>,
                                        std::shared_ptr<std::vector<char>>,
                                        std::function<bool(const std::string&)>,
                                        std::integral_constant<unsigned long long, 0xb00b>
                                    >,
                                    std::variant<
                                        std::array<int, 0xd>,
                                        std::array<int, 0xe>,
                                        std::array<int, 0xa>,
                                        std::array<int, 0xd>,
                                        decltype([]<class T>(
                                            T&& homosexual) -> decltype(auto)
                                            {
                                                return (homosexual);
                                            })
                                    >
                                >,
                                0xa
                            >
                        >
                    >
                >
            >,
            0xd
        >,
        std::conditional_t<
            true,
            std::tuple<
                std::function<
                    std::vector<std::string>(
                        std::list<int>,
                        std::forward_list<long>,
                        std::initializer_list<char>)
                >,
                std::array<
                    std::pair<
                        std::map<
                            std::string,
                            std::variant<
                                std::set<int>,
                                std::set<long>,
                                std::set<long long>,
                                std::set<unsigned>,
                                std::set<unsigned long>,
                                std::set<unsigned long long>
                            >
                        >,
                        std::deque<
                            std::tuple<
                                std::nullptr_t,
                                std::byte,
                                bool,
                                char,
                                wchar_t,
                                char8_t,
                                char16_t,
                                char32_t>
                        >
                    >,
                    3
                >,
                decltype([]() { return []() { return []() { return 0; }; }; })
            >,
            std::tuple<>
        >,
        std::conditional_t<
            true,
            std::tuple<
                std::variant<
                    std::pair<
                        decltype([] { return 0xb; }),
                        decltype([] { return 0x0; })
                    >,
                    std::pair<
                        decltype([] { return 0x0; }),
                        decltype([] { return 0xb; })
                    >,
                    std::pair<
                        decltype([] { return 0x1; }),
                        decltype([] { return 0xe; })
                    >,
                    std::pair<
                        decltype([] { return 0x5; }),
                        decltype([] { return 0x5; })
                    >
                >,
                std::array<
                    std::tuple<
                        std::remove_const_t<const int>,
                        std::remove_volatile_t<volatile long>,
                        std::remove_cv_t<const volatile short>,
                        std::remove_reference_t<int&>,
                        std::remove_cvref_t<const volatile long&>,
                        std::type_identity_t<unsigned>
                    >,
                    676767
                >,
                std::map<
                    std::string,
                    std::vector<
                        std::pair<
                            std::size_t,
                            std::tuple<
                                std::chrono::duration<long long, std::ratio<55555>>,
                                std::chrono::duration<long long, std::ratio<60>>,
                                std::chrono::duration<long long, std::ratio<3600>>
                            >
                        >
                    >
                >
            >,
            std::tuple<
                std::array<
                    std::pair<
                        std::string,
                        std::variant<
                            std::set<int>,
                            std::set<long>,
                            std::set<long long>,
                            std::set<unsigned>,
                            std::set<unsigned long>,
                            std::set<unsigned long long>
                        >
                    >,
                    3
                >
            >
        >
    >
>;

bool setupTitleLogoReplacement(CCSprite* titleLogo)
{
    auto limitLabelWidth = [](CCLabelBMFont* label, const float width, const float defaultScale,
                              const float minScale)
    {
        if (!label) return false;

        const float originalWidth = label->getContentSize().width;
        const float currentScale = label->getScale();
        const float requested = (defaultScale > 0.0f) ? defaultScale : currentScale;

        if (originalWidth <= 0.0f || width <= 0.0f)
        {
            label->setScale(requested);
            return true;
        }

        const float maxAllowed = width / originalWidth;
        float finalScale = requested;

        if (requested > 1.0f)
        {
            if (originalWidth * requested <= width) finalScale = requested;
            else finalScale = std::min(requested, maxAllowed);
        }
        else
        {
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

    if (Mod::get()->getSettingValue<bool>("force-uppercase"))
        std::ranges::transform(
            temp.begin(), temp.end(), temp.begin(), [](const unsigned char c)
            {
                return static_cast<char>(std::toupper(c));
            });

    CCLabelBMFont* newTitleLogo = CCLabelBMFont::create(temp.c_str(), "merged_output.fnt"_spr);
    CCLabelBMFont* newTitleLogoUnderlay = CCLabelBMFont::create(
        temp.c_str(), "underlay_bigsheet.fnt"_spr);
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
    newTitleLogo->setPosition(newTitleLogo->getPositionX() + 6.f,
                              newTitleLogo->getPositionY() - 4.f);
    newTitleLogoUnderlay->setPosition(newTitleLogo->getPosition());

    return true;
}

class $modify(TLTLoadingLayer, LoadingLayer)
{
    // struct Fields
    // {
    //     EventListener<web::Web> m_listener;
    // };

    [[maybe_unused]] static void onModify(auto& self)
    {
        if (const Result<> res = self.
            setHookPriorityAfterPost("LoadingLayer::init", "bitz.darkmode_v4"); !res || res.isErr())
            log::error("could not set prio: {}", res.unwrapErr());
    }

    $override bool init(const bool fromRefresh)
    {
        errorCode.store(0);

        auto rawResponsePtr = std::make_shared<std::string>(".");


        if (!fromRefresh)
        {
#ifdef gay
            const auto savedCache = Mod::get()->getSavedValue<std::string>("cached-ewd-string");
            {
                // populate cache
                std::scoped_lock lock(cachedMutex);
                if (!savedCache.empty()) cachedEWDString = savedCache;
            }

            if (savedCache.empty()) errorCode.fetch_add(1);
            else *rawResponsePtr = savedCache;

            if (Mod::get()->getSettingValue<bool>("aggressive-prefetch"))
            {
                auto req = web::WebRequest().timeout(std::chrono::seconds(3));
                web::WebTask etask = req.get(
                    "https://raw.githubusercontent.com/AnhNguyenlost13/every-word-dash-api/refs/heads/master/badeline.txt");

                constexpr auto maxWait = std::chrono::milliseconds(3000);
                auto waited = std::chrono::milliseconds(0);
                constexpr auto step = std::chrono::milliseconds(1);
                while (etask.isPending() && waited < maxWait)
                {
                    std::this_thread::sleep_for(step);
                    waited += step;
                }

                if (const auto finished = etask.getFinishedValue())
                {
                    *rawResponsePtr = finished->string().unwrapOr(".");
                    if ((*rawResponsePtr) == ".")errorCode.fetch_add(1);
                }
                else errorCode.fetch_add(1);
            }
            else
            {
                m_fields->m_listener.bind([rawResponsePtr](web::WebTask::Event* event) mutable
                {
                    if (const web::WebResponse* response = event->getValue())
                    {
                        *rawResponsePtr = response->string().unwrapOr(".");

                        if (!rawResponsePtr->empty() && *rawResponsePtr != ".")
                        {
                            std::scoped_lock lock(cachedMutex);
                            cachedEWDString = *rawResponsePtr;
                            Mod::get()->setSavedValue("cached-ewd-string", cachedEWDString);
                        }
                    }
                    else if (event->isCancelled()) errorCode.fetch_add(2);
                });

                m_fields->m_listener.setFilter(web::WebRequest().get(
                    "https://raw.githubusercontent.com/AnhNguyenlost13/every-word-dash-api/refs/heads/master/badeline.txt"));

                if ((*rawResponsePtr) == "." || rawResponsePtr->empty()) errorCode.fetch_add(4);
                else
                {
                    std::scoped_lock lock(cachedMutex);
                    cachedEWDString = *rawResponsePtr;
                }
            }

            {
                std::scoped_lock lock(cachedMutex);
                if ((*rawResponsePtr) != cachedEWDString && !rawResponsePtr->empty() && (*
                    rawResponsePtr) != ".")
                {
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
                std::ranges::transform(upper.begin(), upper.end(), upper.begin(),
                                       [](unsigned char c)
                                       {
                                           return static_cast<char>(std::toupper(c));
                                       });
                {
                    std::scoped_lock lock(cachedMutex);
                    cachedEWDString = std::move(upper);
                }
            }
#endif

            CCFileUtils::sharedFileUtils()->addSearchPath(
                (Mod::get()->getTempDir() / "resources").string().c_str());

            CCTextureCache* textureCache = CCTextureCache::sharedTextureCache();
            // preload
            auto tex1 = textureCache->addImage("merged_atlas.png"_spr, false);
            auto tex2 = textureCache->addImage("underlay_atlas.png"_spr, false);

            // more preload?
            auto labelA = CCLabelBMFont::create("trans rights are human rights",
                                                "merged_output.fnt"_spr);
            auto labelB = CCLabelBMFont::create(
                "if you're curious why I have to create these labels, uhhh idk either lol rob does it so do I",
                "underlay_bigsheet.fnt"_spr);
        }

        if (Mod::get()->getSettingValue<bool>("every-word-dash-integration") && dysphoria)
        {
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

class $modify(TLTMenuLayer, MenuLayer)
{
    $override bool init() override
    {
        if (!MenuLayer::init()) return false;

        const auto titleLogo = typeinfo_cast<CCSprite*>(getChildByIDRecursive("main-title"));
        if (!titleLogo) return true;

        if (!Mod::get()->getSettingValue<bool>("every-word-dash-integration") || !dysphoria)
            customTitleLogo =
                Mod::get()->getSettingValue<std::string>("custom-title-logo");
        else
        {
            if (errorCode.load() > 1)
            {
                if (errorCode.load() == 5)
                {
                    Notification::create(
                        "Restart the game for the latest word, sorry!",
                        NotificationIcon::Info,
                        1.f
                    )->show();
                }
                else
                {
                    Notification::create(
                        fmt::format("Failed to fetch/parse EWD string (errcode {})",
                                    errorCode.load()),
                        NotificationIcon::Error,
                        1.f
                    )->show();
                }
            }
            else
            {
                std::scoped_lock lock(cachedMutex);
                customTitleLogo = cachedEWDString;
            }
        }

        setupTitleLogoReplacement(titleLogo);
        return true;
    }
};
