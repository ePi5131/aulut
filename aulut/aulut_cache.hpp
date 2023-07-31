#pragma once
#include <unordered_map>
#include <filesystem>

#include <luawrap.hpp>
#include <cube.hpp>

//#define SIMDEN_EMULATE_INTRINSICS
//#define CUBEHPP_PARALLEL_OFF
#include <cube_simd.hpp>

namespace aulut {
	inline auto my_cube_from_file(const std::filesystem::path& path) {
#if AULUT_INTRINSICS_MODE == AULUT_INTRINSICS_MODE_SSE
		return CubeHpp::cube_bgra32_from_file(path);
#elif AULUT_INTRINSICS_MODE == AULUT_INTRINSICS_MODE_AVX2
		return CubeHpp::cube_bgra32_avx2_from_file(path);
#elif AULUT_INTRINSICS_MODE == AULUT_INTRINSICS_MODE_AVX512
		return CubeHpp::cube_bgra32_avx512_from_file(path);
#endif
	}

	using lut_t = std::invoke_result_t<decltype(my_cube_from_file), std::filesystem::path>;
	using cache_t = std::unordered_map<std::filesystem::path, lut_t>;
	using itr_t = cache_t::const_iterator;

	inline auto& get_cache() {
		static cache_t map{};
		return map;
	}

	inline int itr_t_index(lua_State* _l) {
		Lua::State L{_l};
		using Lua::Type;

		try {
			const auto itrp = L[1].toClass<itr_t>();
			if (!itrp) {
				throw Lua::Exception{ "Unexpected self (aulut_data)" };
			}
			const auto& lut = (*itrp)->second;

			switch (L[2].type()) {
				case Type::Number: {
					const auto idx = L[2].toInteger();

					const auto& domain = lut.at(idx);
					L.newTable(3, 0);
					L.pushNumber(domain[0]);
					L.rawSet(-2, 1);
					L.pushNumber(domain[1]);
					L.rawSet(-2, 2);
					L.pushNumber(domain[2]);
					L.rawSet(-2, 3);
					return 1;
				}
				case Type::String: {
					auto idx = L[2].toStringTrusted();

					if (auto cmp = idx[0] <=> 's'; cmp < 0) {
						if (!std::ranges::equal(idx, "convert")) return 0;
						// convert

						L.pushCFunction([](lua_State* l) {
							Lua::State L{l};
							const auto itrp = L[1].toClass<itr_t>();
							if (!itrp) {
								throw Lua::Exception{ "Unexpected self (aulut_data)" };
							}
							const auto& lut = (*itrp)->second;

							CubeHpp::Domain d = {
								static_cast<float>(L[2].toNumber()),
								static_cast<float>(L[3].toNumber()),
								static_cast<float>(L[4].toNumber())
							};

							try {
								if (auto af = L[5].optNumber()) {
									lut.apply(d, std::clamp(static_cast<float>(af.value()), 0.f, 1.f));
								}
								else {
									lut.apply(d);
								}
							}
							catch (CubeHpp::cube_exception& e) {
								throw Lua::Exception{ e.what() };
							}

							L.pushNumber(d[0]);
							L.pushNumber(d[1]);
							L.pushNumber(d[2]);
							return 3;
						});
					}
					else if (cmp > 0) {
						if (idx[0] != 't') return 0;

						if (idx[1] == 'i') {
							if (!std::ranges::equal(idx.substr(2), "tle")) return 0;
							// title

							if (const auto& title = lut.title()) L.pushString(title.value());
							else L.pushNil();
						}
						else if (idx[1] == 'y') {
							if (!std::ranges::equal(idx.substr(2), "pe")) return 0;
							// type

							L.pushInteger(std::to_underlying(lut.type()));
						}
						else return 0;
					}
					else {
						if (!std::ranges::equal(idx.substr(1), "ize")) return 0;
						// size

						L.pushInteger(lut.size());
					};

					return 1;
				}
				default:
					return 0;
			}
		}
		catch (Lua::Exception& e) {
			L.push(e);
		}
		L.error();
	}
}

namespace Lua {
	template<> void luaClassRequire<aulut::itr_t>(State L) noexcept {
		L.emplaceField("__index", aulut::itr_t_index);
	}
	template<> const char* luaClassName<aulut::itr_t>(State) noexcept {
		return "aulut_data";
	}
}
