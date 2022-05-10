#pragma once
#include <entt/entt.hpp>

namespace SolsticeGE {

	/// <summary>
	/// What thread should a
	/// system be updated on?
	/// </summary>
	enum class SystemThread {
		SYS_GAMETHREAD,
		SYS_RENDERTHREAD,
		SYS_BACKGROUND
	};

	class System
	{
	public:

		inline System() : threadType(SystemThread::SYS_GAMETHREAD) {};
		inline ~System() {};

		/// <summary>
		/// update function of the system,
		/// the virtual function call isn't a
		/// performance issue because each system
		/// will be called once to handle all entities
		/// that system is responsible for
		/// </summary>
		/// <param name="registry">the ECS registry</param>
		virtual void update(entt::registry& registry) = 0;
	
	protected:

		SystemThread threadType;
	};

}

