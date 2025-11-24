#pragma once

#include <functional>
#include <vector>
#include "Arduino.h"
#include "StreamingOperator.hpp"
#include "Homie/Datatypes/Interface.hpp"
#include "Homie/Datatypes/Callbacks.hpp"
#include "Homie/Limits.hpp"
#include "HomieRange.hpp"

class HomieNode;

namespace HomieInternals {
class HomieClass;
class Property;
class BootNormal;
class BootConfig;
class SendingPromise;


class PropertyInterface {
  friend ::HomieNode;

 public:
  PropertyInterface();

  PropertyInterface& settable(const PropertyInputHandler& inputHandler = [](const HomieRange& range, const String& value) { return false; });

  // setName/setUnit mit optionalem copy-Parameter
  PropertyInterface& setName(const char* name, bool copy = true);
  PropertyInterface& setUnit(const char* unit, bool copy = true);

  PropertyInterface& setDatatype(const char* datatype);
  PropertyInterface& setFormat(const char* format);
  PropertyInterface& setRetained(const bool retained = true);

 private:
  PropertyInterface& setProperty(Property* property);

  Property* _property;
};


class Property {
  friend HomieNode;
  friend BootNormal;
  friend class PropertyInterface;

 public:
  explicit Property(const char* id) {
    _id = strdup(id);

    _name = nullptr;      // kein Speicherverbrauch am Anfang
    _unit = nullptr;      // kein Speicherverbrauch am Anfang
    _name_owned = false;
    _unit_owned = false;

    _datatype = "";       // unverändert
    _format   = "";       // unverändert

    _retained = true;
    _settable = false;
  }

  ~Property() {
    if(_id) free((void*)_id);
    if(_name_owned && _name) free((void*)_name);
    if(_unit_owned && _unit) free((void*)_unit);
  }

  void settable(const PropertyInputHandler& inputHandler) { _settable = true;  _inputHandler = inputHandler; }

  // optional copy für Name
  void setName(const char* name, bool copy = false) { 
    if(_name_owned && _name) free((void*)_name);
    if(copy) {
      _name = strdup(name);
      _name_owned = true;
    } else {
      _name = name;
      _name_owned = false;
    }
  }

  // optional copy für Unit
  void setUnit(const char* unit, bool copy = false) { 
    if(_unit_owned && _unit) free((void*)_unit);
    if(copy) {
      _unit = strdup(unit);
      _unit_owned = true;
    } else {
      _unit = unit;
      _unit_owned = false;
    }
  }

  void setDatatype(const char* datatype) { _datatype = datatype; }
  void setFormat(const char* format) { _format = format; }
  void setRetained(const bool retained = true) { _retained = retained; }

 private:
  const char* getId() const { return _id; }
  const char* getName() const { return _name; }
  const char* getUnit() const { return _unit; }
  const char* getDatatype() const { return _datatype; }
  const char* getFormat() const { return _format; }
  bool isRetained() const { return _retained; }
  bool isSettable() const { return _settable; }
  PropertyInputHandler getInputHandler() const { return _inputHandler; }
  const char* _id;
  const char* _name;
  const char* _unit;
  const char* _datatype;
  const char* _format;
  bool _retained;
  bool _settable;
  PropertyInputHandler _inputHandler;

  // Flags für interne Speicherverwaltung
  bool _name_owned;
  bool _unit_owned;
};



}  // namespace HomieInternals

class HomieNode {
  friend HomieInternals::HomieClass;
  friend HomieInternals::BootNormal;
  friend HomieInternals::BootConfig;

 public:
  HomieNode(const char* id, const char* name, const char* type, bool range = false, uint16_t lower = 0, uint16_t upper = 0, const HomieInternals::NodeInputHandler& nodeInputHandler = [](const HomieRange& range, const String& property, const String& value) { return false; });
  virtual ~HomieNode();

  const char* getId() const { return _id; }
  const char* getType() const { return _type; }
  const char* getName() const {return _name; }
  bool isRange() const { return _range; }
  uint16_t getLower() const { return _lower; }
  uint16_t getUpper() const { return _upper; }

  void setId(const char* id) { _id = id; }
  void setType(const char* type) { _type = type; }
  void setName(const char* name) { _name = name; }

  HomieInternals::PropertyInterface& advertise(const char* id);
  HomieInternals::SendingPromise& setProperty(const String& property) const;
  HomieInternals::Property* getProperty(const String& property) const;

  void setRunLoopDisconnected(bool runLoopDisconnected) {
    this->runLoopDisconnected = runLoopDisconnected;
  }

 protected:
  virtual void setup() {}
  virtual void loop() {}
  virtual void onReadyToOperate() {}
  virtual bool handleInput(const HomieRange& range, const String& property, const String& value);
 
  const std::vector<HomieInternals::Property*>& getProperties() const;
 
  static HomieNode* find(const char* id) {
    for (HomieNode* iNode : HomieNode::nodes) {
      if (strcmp(id, iNode->getId()) == 0) return iNode;
    }

    return 0;
  }

private:


  const char* _id;
  const char* _name;
  const char* _type;
  bool _range;
  uint16_t _lower;
  uint16_t _upper;

  bool runLoopDisconnected;

  std::vector<HomieInternals::Property*> _properties;
  HomieInternals::NodeInputHandler _inputHandler;

  HomieInternals::PropertyInterface _propertyInterface;

  static std::vector<HomieNode*> nodes;
};
