// This file is part of Glest <https://github.com/Glest>
//
// Copyright (C) 2018  The Glest team
//
// Glest is a fork of MegaGlest <https://megaglest.org/>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>

#ifndef _AIRULE_H_
#define _AIRULE_H_

#ifdef WIN32
#    include <winsock2.h>
#    include <winsock.h>
#endif

#include <string>
#include "vec.h"
#include "skill_type.h"
#include "leak_dumper.h"

using std::string;
using Shared::Graphics::Vec2i;

namespace Game {

	class
		Ai;
	class
		AiInterface;
	class
		Unit;
	class
		UnitType;
	class
		ProduceTask;
	class
		BuildTask;
	class
		UpgradeTask;
	class
		ResourceType;

	// =====================================================
	//      class AiRule
	//
	///     An action that the AI will perform periodically
	/// if the test succeeds
	// =====================================================

	// The general structure of the rules for an AI to do a task.
	class
		AiRule {
	protected:
		Ai *
			ai;

	public:
		explicit
			AiRule(Ai * ai);
		virtual ~
			AiRule() {
		}

		virtual int
			getTestInterval() const = 0;     //in milliseconds
		virtual string
			getName() const = 0;

		virtual bool
			test() = 0;
		virtual void
			execute() = 0;
	};

	// =====================================================
	//      class AiRuleWorkerHarvest
	// =====================================================

	// The rules for AI tasks inherit from the generic AiRule class.
	class
		AiRuleWorkerHarvest :
		public
		AiRule {
	private:
		int
			stoppedWorkerIndex;

	public:
		explicit
			AiRuleWorkerHarvest(Ai * ai);
		// This value returned byt getTestInterval()
		// is called in ai.cpp to determine the chance the rule is executed.
		virtual int
			getTestInterval() const {
			return
				1000;
		}
		virtual string
			getName() const {
			return
				"Worker stopped => Order worker to harvest";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleRefreshHarvester
	// =====================================================

	class
		AiRuleRefreshHarvester :
		public
		AiRule {
	private:
		int
			workerIndex;

	public:
		explicit
			AiRuleRefreshHarvester(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				20000;
		}
		virtual string
			getName() const {
			return
				"Worker reassigned to needed resource";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleScoutPatrol
	// =====================================================

	class
		AiRuleScoutPatrol :
		public
		AiRule {
	public:
		explicit
			AiRuleScoutPatrol(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				10000;
		}
		virtual string
			getName() const {
			return
				"Base is stable => Send scout patrol";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleRepair
	// =====================================================

	class
		AiRuleRepair :
		public
		AiRule {
	private:
		int
			damagedUnitIndex;
		bool
			damagedUnitIsCastle;

		int
			getMinUnitsToRepairCastle();
		double
			getMinCastleHpRatio() const;

	public:
		explicit
			AiRuleRepair(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				10000;
		}
		virtual string
			getName() const {
			return
				"Building Damaged => Repair";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleReturnBase
	// =====================================================

	class
		AiRuleReturnBase :
		public
		AiRule {
	private:
		int
			stoppedUnitIndex;
	public:
		explicit
			AiRuleReturnBase(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				5000;
		}
		virtual string
			getName() const {
			return
				"Stopped unit => Order return base";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleMassiveAttack
	// =====================================================

	class
		AiRuleMassiveAttack :
		public
		AiRule {
	private:
		static const int
			baseRadius = 25;

	private:
		Vec2i
			attackPos;
		Field
			field;
		bool
			ultraAttack;

	public:
		explicit
			AiRuleMassiveAttack(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				1000;
		}
		virtual string
			getName() const {
			return
				"Unit under attack => Order massive attack";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleAddTasks
	// =====================================================

	class
		AiRuleAddTasks :
		public
		AiRule {
	public:
		explicit
			AiRuleAddTasks(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				5000;
		}
		virtual string
			getName() const {
			return
				"Tasks empty => Add tasks";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleBuildOneFarm
	// =====================================================

	class
		AiRuleBuildOneFarm :
		public
		AiRule {
	private:
		const UnitType *
			farm;

	public:
		explicit
			AiRuleBuildOneFarm(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				10000;
		}
		virtual string
			getName() const {
			return
				"No farms => Build one";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleProduceResourceProducer
	// =====================================================

	class
		AiRuleProduceResourceProducer :
		public
		AiRule {
	private:
		static const int
			minStaticResources = 20;
		static const int
			longInterval = 60000;
		static const int
			shortInterval = 5000;
		const ResourceType *
			rt;
		int
			interval;
		bool
			newResourceBehaviour;

	public:
		explicit
			AiRuleProduceResourceProducer(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				interval;
		}
		virtual string
			getName() const {
			return
				"No resources => Build Resource Producer";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleProduce
	// =====================================================

	class
		AiRuleProduce :
		public
		AiRule {
	private:
		const ProduceTask *
			produceTask;

		typedef
			vector < const UnitType *>
			UnitTypes;
		typedef
			vector <
			bool >
			UnitTypesGiveBack;
		bool
			newResourceBehaviour;

	public:
		explicit
			AiRuleProduce(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				2000;
		}
		virtual string
			getName() const {
			return
				"Performing produce task";
		}

		virtual bool
			test();
		virtual void
			execute();

	private:
		void
			produceGeneric(const ProduceTask * pt);
		void
			produceGenericNew(const ProduceTask * pt);
		void
			produceSpecific(const ProduceTask * pt);
		bool
			canUnitTypeOfferResourceType(const UnitType * ut,
				const ResourceType * rt);
		bool
			setAIProduceTaskForResourceType(const ProduceTask * pt,
				AiInterface * aiInterface);
		void
			addUnitTypeToCandidates(const UnitType * producedUnit,
				UnitTypes & ableUnits,
				UnitTypesGiveBack & ableUnitsGiveBack,
				bool unitCanGiveBackResource);
	};
	// =====================================================
	//      class AiRuleBuild
	// =====================================================

	class
		AiRuleBuild :
		public
		AiRule {
	private:
		const BuildTask *
			buildTask;

	public:
		explicit
			AiRuleBuild(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				1000;
		}
		virtual string
			getName() const {
			return
				"Performing build task";
		}

		virtual bool
			test();
		virtual void
			execute();

	private:
		void
			buildGeneric(const BuildTask * bt);
		void
			buildSpecific(const BuildTask * bt);
		void
			buildBestBuilding(const vector < const UnitType * >&buildings);

		bool
			isDefensive(const UnitType * building);
		bool
			isResourceProducer(const UnitType * building);
		bool
			isWarriorProducer(const UnitType * building);
	};

	// =====================================================
	//      class AiRuleUpgrade
	// =====================================================

	class
		AiRuleUpgrade :
		public
		AiRule {
	private:
		const UpgradeTask *
			upgradeTask;

	public:
		explicit
			AiRuleUpgrade(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				2000;
		}
		virtual string
			getName() const {
			return
				"Performing upgrade task";
		}

		virtual bool
			test();
		virtual void
			execute();

	private:
		void
			upgradeSpecific(const UpgradeTask * upgt);
		void
			upgradeGeneric(const UpgradeTask * upgt);
	};

	// =====================================================
	//      class AiRuleExpand
	// =====================================================

	class
		AiRuleExpand :
		public
		AiRule {
	private:
		static const int
			expandDistance = 30;

	private:
		Vec2i
			expandPos;
		const UnitType *
			storeType;

	public:
		explicit
			AiRuleExpand(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				30000;
		}
		virtual string
			getName() const {
			return
				"Expanding";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

	// =====================================================
	//      class AiRuleUnBlock
	// =====================================================

	class
		AiRuleUnBlock :
		public
		AiRule {
	public:
		explicit
			AiRuleUnBlock(Ai * ai);

		virtual int
			getTestInterval() const {
			return
				3000;
		}
		virtual string
			getName() const {
			return
				"Blocked Units => Move surrounding units";
		}

		virtual bool
			test();
		virtual void
			execute();
	};

} //end namespace

#endif
