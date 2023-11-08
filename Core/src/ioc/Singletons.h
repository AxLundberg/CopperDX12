#pragma once
#include <any>
#include <format>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <functional>
#include <unordered_map>

#include "Container.h"
#include "Exception.h"
#include <Core/src/utl/Assert.h>
#include <Core/src/utl/String.h>

namespace CPR::IOC
{
	class Singletons
	{
	public:
		// types
		template<class T>
		using Generator = std::function<std::shared_ptr<T>()>;
		template<class T>
		using ParameterizedGenerator = std::function<std::shared_ptr<T>(typename T::IocParams)>;
		// functions
		template<Parameterized T>
		void Register(ParameterizedGenerator<T> gen)
		{
			mServiceMap[typeid(T)] = gen;
		}
		template<NotParameterized T>
		void Register(Generator<T> gen)
		{
			mServiceMap[typeid(T)] = gen;
		}
		template<class T>
		void RegisterPassThrough()
		{
			Register<T>([] { return IOC::Get().Resolve<T>(); });
		}
		template<Parameterized T>
		std::shared_ptr<T> Resolve(typename T::IocParams&& params = {})
		{
			return Resolve_<T, ParameterizedGenerator<T>>(std::forward<typename T::IocParams>(params));
		}
		template<NotParameterized T>
		std::shared_ptr<T> Resolve()
		{
			return Resolve_<T, Generator<T>>();
		}
	private:
		template<class T, class G, typename...Ps>
		std::shared_ptr<T> Resolve_(Ps&&...arg)
		{
			// TODO: pull this out of template/header
			if (const auto i = mServiceMap.find(typeid(T)); i != mServiceMap.end())
			{
				auto& entry = i->second;
				try {
					// first check if we have an existing instance, return if so
					if (auto ppInstance = std::any_cast<std::shared_ptr<T>>(&entry)) {
						return *ppInstance;
					}
					// if not, generate instance, store, and return
					auto pInstance = std::any_cast<G>(entry)(std::forward<Ps>(arg)...);
					entry = pInstance;
					return pInstance;
				}
				catch (const std::bad_any_cast&)
				{
					cpr_check_fail.Msg(std::format(
						L"Failed to resolve Singleton mapped type\nFrom: [{}]\n to: [{}]",
						UTL::ToWide(entry.type().name()), UTL::ToWide(typeid(Generator<T>).name())
					)).Ex();
				}

			}
			else
			{
				throw ServiceNotFound{ std::format(
					"Failed to find entry for type [{}] in singleton container",
					typeid(T).name())
				};
			}
		}
	private:
		// data
		std::unordered_map<std::type_index, std::any> mServiceMap;
	};

	Singletons& Sing();
}
