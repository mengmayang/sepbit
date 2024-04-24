#include "src/logstore/scheduler/scheduler1.h"
#include "src/logstore/scheduler/scheduler2.h"
#include "src/logstore/scheduler/scheduler.h"
#include "src/logstore/manager.h"
#include <iostream>

class SchedulerFactory {
  public:
    static Scheduler *GetInstance(std::string type, Manager *manager) {
      std::cout << "Scheduler algorithm: " << type << std::endl;
      if (type == "raw") {
        return new Scheduler1(manager);
      } else if (type == "new") {
        return new Scheduler2(manager);
      } else {
        std::cerr << "No Selection, type: " << type << std::endl;
      }
      return new Scheduler1(manager);
    }
};

