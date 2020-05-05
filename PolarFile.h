#pragma once
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <memory>

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

namespace Sailing::PolarFiles {

    using str_ptr = std::shared_ptr<std::string>;

    using FloatPair = std::pair<float, float>;
    using FloatPair_ptr = std::shared_ptr<FloatPair>;
    using TwsCurve = std::vector<FloatPair_ptr>;
    using TwsCurve_ptr = std::shared_ptr<TwsCurve>;


    class PolarFile {
    public:
        void import_polar(std::string const& input_polar_fn);
        void export_polar(std::string const& output_polar_fn);

    private:
        std::string_view table_deco0 { "TWA\\TWS"sv };
        std::string_view table_deco1 { "TWA/TWS"sv };
        std::string_view table_header { "!pqPolarGenerator Exd format -- tws curves"sv };

        std::shared_ptr<std::vector<float>> tws_list {};

        std::vector<float> twa_list {};
        std::map<float, std::shared_ptr<std::vector<float>>> old_polar_map {};

        std::shared_ptr<std::vector<TwsCurve_ptr>> polar;

        std::vector<str_ptr> break_strings(str_ptr input, char const cut_char);
        std::vector<float> break_strings_float(str_ptr input, char const cut_char);
        TwsCurve_ptr build_pairs(std::vector<float> const& tws, std::vector<float> const& bsp);
        bool my_begins_with(std::string_view const str, std::string_view const prefix);
    };



    enum class SailingUnits {
        TWA,
        TWD,
        TWS,
        AWA,
        AWD,
        AWS,
        BSP,
        GSP
    };

}