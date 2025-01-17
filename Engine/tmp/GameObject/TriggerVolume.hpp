#ifndef TRIGGER_VOLUME_H
#define TRIGGER_VOLUME_H

#include "Engine/GameObject/GameObject.hpp"

class TriggerVolume : public GameObject {
public:
    TriggerVolume();

    virtual void Create(jsonObj node) override;
    virtual void PostLoad() override;
    virtual void Update(double dt) override;

    virtual const char* ObjectTypeString() override;

    inline bool Inside() { return inside; }

    vec3 bounds_min, bounds_max;

protected:
    //GameObject *m_triggerObject;
    UID_t m_triggerObjectID;
    bool inside;

private:
    bool pointInsideBox(vec3 p);

    static const char* _obj_type_TriggerVolume;

    std::string m_triggerObjectName;
};

#endif
