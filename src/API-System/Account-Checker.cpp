#include <cstdio>
#include <cstring>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <memory>
#include <utility>

enum class EvidenceType {
    LOGIN_PAGE,
    CAPTCHA,
    SOFT_404,
    PROFILE_HINT,
    BLOCKED,
    REDIRECT,
    UNKNOWN
};

struct Evidence {
    EvidenceType type;
    std::string note;
};

struct AnalysisResult {
    int http_status;
    std::vector<Evidence> evidence;
    double score = 0.0;
    enum class UserExistence {
        EXISTS,
        NOT_FOUND,
        UNKNOWN
    } existence = UserExistence::UNKNOWN;
    std::string error;
};

struct FetchResponse {
    int status_code = 0;
    std::string body;
    std::string error;
};

std::string shell_quote(const std::string& value) {
    std::string quoted = "'";
    for (char character : value) {
        if (character == '\'') {
            quoted += "'\\\"'\\\"'";
        } else {
            quoted += character;
        }
    }
    return quoted + "'";
}

FetchResponse fetch_url(const std::string& url, const std::string& bearer_token_env = "") {
    constexpr const char* marker = "__ACCOUNT_CHECK_HTTP_STATUS__";
    const std::string authorization_header = bearer_token_env.empty()
        ? ""
        : "--header \"Authorization: Bearer $" + bearer_token_env + "\" ";
    const std::string command =
        "curl --silent --show-error --location --max-time 10 "
        "--user-agent 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/120' "
        "--header 'Accept: text/html' "
        "--header 'Accept-Language: en-US,en;q=0.9' "
        + authorization_header +
        "--write-out '\\n" + std::string(marker) + "%{http_code}' " +
        shell_quote(url) + " 2>&1";

    FetchResponse response;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        response.error = "Could not start curl";
        return response;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        response.body += buffer;
    }
    const int exit_status = pclose(pipe);
    const std::size_t marker_pos = response.body.rfind(marker);
    if (marker_pos != std::string::npos) {
        const std::string status = response.body.substr(marker_pos + std::strlen(marker));
        response.body.erase(marker_pos);
        try {
            response.status_code = std::stoi(status);
        } catch (const std::exception&) {
            response.error = "curl returned an invalid HTTP status";
        }
    }
    if (exit_status != 0 && response.error.empty()) {
        const int curl_exit_code = WIFEXITED(exit_status) ? WEXITSTATUS(exit_status) : exit_status;
        response.error = "curl failed with exit code " + std::to_string(curl_exit_code);
    }
    return response;
}

std::string x_handle_from_url(const std::string& url) {
    const std::size_t scheme_end = url.find("://");
    const std::size_t host_start = scheme_end == std::string::npos ? 0 : scheme_end + 3;
    const std::size_t path_start = url.find('/', host_start);
    if (path_start == std::string::npos) {
        return "";
    }
    const std::size_t handle_end = url.find_first_of("/?#", path_start + 1);
    std::string handle = url.substr(path_start + 1, handle_end - path_start - 1);
    if (!handle.empty() && handle.front() == '@') {
        handle.erase(0, 1);
    }
    if (handle.empty() || handle.size() > 15 || handle == "home" || handle == "search" ||
        handle == "i" || handle == "intent" || handle == "share") {
        return "";
    }
    return std::all_of(handle.begin(), handle.end(), [](unsigned char character) {
        return std::isalnum(character) || character == '_';
    }) ? handle : "";
}


struct SignatureRule {
    std::vector<std::string> substrings;
    EvidenceType type;
    std::string note;
    double score_modifier;
    std::vector<std::string> required_substrings;

    SignatureRule(std::vector<std::string> optional_substrings, EvidenceType evidence_type,
                  std::string evidence_note, double modifier,
                  std::vector<std::string> required = {})
        : substrings(std::move(optional_substrings)), type(evidence_type),
          note(std::move(evidence_note)), score_modifier(modifier),
          required_substrings(std::move(required)) {}
};

struct SiteConfig {
    std::string site_name;
    std::string url_pattern; 
    std::vector<SignatureRule> rules;
};

class UniversalAnalyzer {
private:
    std::vector<SiteConfig> site_configs;
    SiteConfig default_config;
    const SiteConfig& get_config_for_url(const std::string& url) const {
        for (const auto& config : site_configs) {
            if (url.find(config.url_pattern) != std::string::npos) {
                return config;
            }
        }
        return default_config;
    }

public:
    UniversalAnalyzer() {
        setup_default_config();
        setup_site_specific_configs();
    }

    void register_site(const SiteConfig& config) {
        site_configs.push_back(config);
    }

    AnalysisResult analyze(const std::string& url) const {
        AnalysisResult result;
        const SiteConfig& config = get_config_for_url(url);
        const FetchResponse response = fetch_url(url);
        result.http_status = response.status_code;
        result.error = response.error;

        std::string page = response.body;
        std::transform(page.begin(), page.end(), page.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });

        double calculated_score = 0.0;
        bool has_profile_hint = false;
        for (const auto& rule : config.rules) {
            bool rule_matches = rule.substrings.empty();
            for (const auto& substr : rule.substrings) {
                std::string normalized_substr = substr;
                std::transform(normalized_substr.begin(), normalized_substr.end(), normalized_substr.begin(), [](unsigned char character) {
                    return static_cast<char>(std::tolower(character));
                });
                if (page.find(normalized_substr) != std::string::npos) {
                    rule_matches = true;
                    break;
                }
            }
            if (rule_matches) {
                for (const auto& required_substr : rule.required_substrings) {
                    std::string normalized_required_substr = required_substr;
                    std::transform(normalized_required_substr.begin(), normalized_required_substr.end(), normalized_required_substr.begin(), [](unsigned char character) {
                        return static_cast<char>(std::tolower(character));
                    });
                    if (page.find(normalized_required_substr) == std::string::npos) {
                        rule_matches = false;
                        break;
                    }
                }
            }
            if (rule_matches) {
                result.evidence.push_back({rule.type, rule.note});
                calculated_score += rule.score_modifier;
                has_profile_hint = has_profile_hint || rule.type == EvidenceType::PROFILE_HINT;
            }
        }

        const bool is_x_profile = config.site_name == "X / Twitter";
        if (is_x_profile && !has_profile_hint) {
            const std::string handle = x_handle_from_url(url);
            if (!handle.empty() && std::getenv("X_BEARER_TOKEN") != nullptr) {
                const FetchResponse api_response = fetch_url(
                    "https://api.x.com/2/users/by/username/" + handle, "X_BEARER_TOKEN");
                if (api_response.status_code == 200 && api_response.body.find("\"data\"") != std::string::npos) {
                    result.http_status = api_response.status_code;
                    result.error = api_response.error;
                    result.evidence.push_back({EvidenceType::PROFILE_HINT, "X API user lookup succeeded"});
                    calculated_score += 0.8;
                    has_profile_hint = true;
                } else if (api_response.status_code == 404) {
                    result.http_status = api_response.status_code;
                    result.error = api_response.error;
                    result.evidence.push_back({EvidenceType::SOFT_404, "X API reports that this username was not found"});
                    calculated_score += 0.2;
                }
            } else if (!handle.empty()) {
                result.evidence.push_back({EvidenceType::LOGIN_PAGE,
                    "X returned a public-page shell; set X_BEARER_TOKEN for a definitive lookup"});
            }
        }

        if (response.status_code == 400 || response.status_code == 403) {
            result.evidence.push_back({EvidenceType::BLOCKED, "Request explicitly blocked by server"});
            calculated_score -= 0.4;
        }

        const bool has_missing_page_signal = std::any_of(
            result.evidence.begin(), result.evidence.end(), [](const Evidence& evidence) {
                return evidence.type == EvidenceType::SOFT_404;
            });
        if (response.status_code == 404 || response.status_code == 410 || has_missing_page_signal) {
            result.existence = AnalysisResult::UserExistence::NOT_FOUND;
        } else if (response.status_code == 0 || response.status_code == 401 || response.status_code == 403 ||
                   response.status_code == 429 || response.status_code >= 500) {
            result.existence = AnalysisResult::UserExistence::UNKNOWN;
        } else if (response.status_code >= 200 && response.status_code < 300 && has_profile_hint) {
            result.existence = AnalysisResult::UserExistence::EXISTS;
        }

        result.score = std::max(0.0, std::min(1.0, calculated_score));
        return result;
    }

private:
    void setup_default_config() {
        default_config.site_name = "Generic Webpage";
        default_config.url_pattern = "";
        default_config.rules = {
            { {"login", "password", "sign-in", "signin"}, EvidenceType::LOGIN_PAGE, "Generic login wall", 0.1 },
            { {"captcha", "recaptcha", "g-recaptcha", "cloudflare"}, EvidenceType::CAPTCHA, "Generic anti-bot challenge", -0.3 },
            { {"404", "page not found", "profile not found", "user not found", "account not found",
               "this user does not exist", "this account does not exist", "doesn't exist", "does not exist"},
              EvidenceType::SOFT_404, "Generic missing profile indicator", 0.1 },
            { {"og:type\" content=\"profile\"", "og:type' content='profile'", "\"og:type\":\"profile\""},
              EvidenceType::PROFILE_HINT, "Open Graph profile metadata detected", 0.7 },
            { {}, EvidenceType::PROFILE_HINT, "Structured Person profile metadata detected", 0.6,
              {"application/ld+json", "\"@type\":\"person\""} },
            { {}, EvidenceType::PROFILE_HINT, "Structured Person profile metadata detected", 0.6,
              {"application/ld+json", "\"@type\": \"person\""} },
            { {"profilepage", "profile-header", "profile_header", "user-profile", "user_profile"},
              EvidenceType::PROFILE_HINT, "Generic profile page structure detected", 0.45 }
        };
    }

    void setup_site_specific_configs() {
        SiteConfig facebook;
        facebook.site_name = "Facebook";
        facebook.url_pattern = "facebook.com";
        facebook.rules = {
            { {"profile_pic", "Timeline", "userContent", "og:title"}, EvidenceType::PROFILE_HINT, "FB profile structure detected", 0.5 },
            { {"content isn't available", "page isn't available", "not available right now"}, EvidenceType::SOFT_404, "FB Soft Block / Unavailable profile", 0.2 },
        };
        register_site(facebook);
        SiteConfig github;
        github.site_name = "GitHub";
        github.url_pattern = "github.com";
        github.rules = {
            { {"og:type\" content=\"profile\"", "pinned-items-modal", "Contributions"}, EvidenceType::PROFILE_HINT, "GitHub Profile detected", 0.6 },
            { {"Sign in to GitHub"}, EvidenceType::LOGIN_PAGE, "GitHub Login Wall", 0.1 }
        };
        register_site(github);
        SiteConfig instagram;
        instagram.site_name = "Instagram";
        instagram.url_pattern = "instagram.com";
        instagram.rules = {
            { {"og:type\" content=\"profile\"", "profilepage_", "profile_pic_url", "edge_owner_to_timeline_media"}, EvidenceType::PROFILE_HINT, "Instagram profile structure detected", 0.6 },
            { {"sorry, this page isn't available", "the link you followed may be broken", "user not found"}, EvidenceType::SOFT_404, "Instagram unavailable profile", 0.2 },
            { {"login • instagram", "log in to instagram"}, EvidenceType::LOGIN_PAGE, "Instagram login wall", 0.1 }
        };
        register_site(instagram);
        SiteConfig twitter;
        twitter.site_name = "X / Twitter";
        twitter.url_pattern = "twitter.com";
        twitter.rules = {
            { {"data-testid=\"username\"", "profilepage", "\"screen_name\"", "profile_image_url_https"}, EvidenceType::PROFILE_HINT, "X/Twitter profile structure detected", 0.6 },
            { {"this account doesn\xE2\x80\x99t exist", "this account doesn't exist", "this page doesn\xE2\x80\x99t exist", "this page doesn't exist"}, EvidenceType::SOFT_404, "X/Twitter unavailable profile", 0.2 },
            { {"log in to x", "sign in to x"}, EvidenceType::LOGIN_PAGE, "X/Twitter login wall", 0.1 }
        };
        register_site(twitter);

        SiteConfig x;
        x = twitter;
        x.url_pattern = "x.com";
        register_site(x);
        SiteConfig threads;
        threads.site_name = "Threads";
        threads.url_pattern = "threads.com";
        threads.rules = {
            { {"og:type\" content=\"profile\"", "threads.net/@", "profile_pic_url", "\"username\""}, EvidenceType::PROFILE_HINT, "Threads profile structure detected", 0.6 },
            { {"sorry, this page isn't available", "the link you followed may be broken", "user not found"}, EvidenceType::SOFT_404, "Threads unavailable profile", 0.2 },
            { {"log in to threads", "log in with instagram"}, EvidenceType::LOGIN_PAGE, "Threads login wall", 0.1 }
        };
        register_site(threads);
        SiteConfig codeforces;
        codeforces.site_name = "Codeforces";
        codeforces.url_pattern = "codeforces.com";
        codeforces.rules = {
            { {"user profile", "rated user", "unrated user", "contest rating"}, EvidenceType::PROFILE_HINT, "Codeforces profile structure detected", 0.6 },
            { {"handle not found", "user with handle", "not found"}, EvidenceType::SOFT_404, "Codeforces unavailable profile", 0.2 },
            { {"enter"}, EvidenceType::LOGIN_PAGE, "Codeforces login page", 0.1 }
        };
        register_site(codeforces);
    }
};

void print_result(const std::string& url, const AnalysisResult& res) {
    std::cout << "\n========================================\n";
    std::cout << "Target URL: " << url << "\n";
    std::cout << "HTTP Status: " << res.http_status << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Evidence Gathered:\n";

    if (res.evidence.empty()) {
        std::cout << " - No specific signs detected.\n";
    } else {
        for (const auto& e : res.evidence) {
            std::cout << " [!] " << e.note << "\n";
        }
    }

    std::cout << "----------------------------------------\n";
    std::cout << "Confidence Score (0-1): " << res.score << "\n";
    std::cout << "User existence: ";
    switch (res.existence) {
        case AnalysisResult::UserExistence::EXISTS:
            std::cout << "EXISTS\n";
            break;
        case AnalysisResult::UserExistence::NOT_FOUND:
            std::cout << "NOT_FOUND\n";
            break;
        case AnalysisResult::UserExistence::UNKNOWN:
            std::cout << "UNKNOWN\n";
            break;
    }
    if (!res.error.empty())
        std::cout << "Request error: " << res.error << "\n";
    std::cout << "========================================\n";
}

int main() {
    UniversalAnalyzer analyzer;
    std::string url;
    std::cout << "Enter target URL to analyze: ";
    std::cin >> url;
    AnalysisResult res = analyzer.analyze(url);
    print_result(url, res);
    return 0;
}
