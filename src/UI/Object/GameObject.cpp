//
// Created by zhou_zhengming on 2026/5/17.
//

#include "UI/Object/GameObject.h"
#include "UI/Material/DefaultMaterial.h"

z8::GameObject::GameObject(): Mesh(nullptr), Collider(nullptr), Material(new DefaultMaterial())
{

}
