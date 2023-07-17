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
    using StrVec = std::vector<str_ptr>;

    using FloatPair = std::pair<float, float>;
    using FloatPair_ptr = std::shared_ptr<FloatPair>;
    using FloatVec = std::vector<float>;
    using TwsCurve = std::vector<FloatPair_ptr>;
    using TwsCurve_ptr = std::shared_ptr<TwsCurve>;
    using TwsCurveVec = std::vector<TwsCurve_ptr>;

    using OldPolarMap = std::map<float, std::shared_ptr<FloatVec>>;

    struct OldPolar {
        OldPolar() : twa_list { std::make_shared<FloatVec>() }, old_polar_map { std::make_shared<OldPolarMap>() } {}

        std::shared_ptr<FloatVec> twa_list {};
        std::shared_ptr<OldPolarMap> old_polar_map {};
    };

    class PolarFile {
    public:
        PolarFile() : comments { std::make_shared<StrVec>() }, tws_list { std::make_shared<FloatVec>() }, polar { std::make_shared<std::vector<TwsCurve_ptr>>() } {}


        void import_polar(std::string const& input_polar_fn);
        void read_polar(std::string const& input_polar_fn);
        void export_polar(std::string const& output_polar_fn);

        void normalise_polar();

    private:
        std::string_view table_deco0 { "TWA\\TWS"sv };
        std::string_view table_deco1 { "TWA/TWS"sv };
        std::string_view table_header { "!pqPolarGenerator Exd format -- tws curves"sv };

        //std::shared_ptr<OldPolar> old_polar {nullptr};

        std::shared_ptr<std::vector<str_ptr>> comments {nullptr};

        std::shared_ptr<FloatVec> tws_list {nullptr};

        std::shared_ptr<TwsCurveVec> polar {nullptr};

        StrVec break_strings(str_ptr input, char const cut_char);
        FloatVec break_strings_float(str_ptr input, char const cut_char);
        TwsCurve_ptr build_pairs(std::vector<float> const& tws, std::vector<float> const& bsp);
        bool my_begins_with(std::string_view const str, std::string_view const prefix);
        std::string polar_entry_txt(FloatPair_ptr fp);
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
