#pragma once

#ifdef DEBUG
#define NAME_D3D12_OBJECT(obj, name) obj->SetName(name);
#else
#define NAME_D3D12_OBJECT(obj, name)
#endif // DEBUG

#ifdef DEBUG
#define _DEBUG_ true
#else
#define _DEBUG_ false
#endif // DEBUG
