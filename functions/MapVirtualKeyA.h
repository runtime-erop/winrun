#include "../lib.h"

UINT MapVirtualKeyA(UINT uCode, UINT uMapType){
    
    switch (uMapType)
    {
    case MAPVK_VK_TO_VSC:
        return winapi_to_linux_key(uCode);
    case MAPVK_VSC_TO_VK:
        return linux_to_winapi_key(uCode);
    case MAPVK_VK_TO_CHAR:
        return uCode;
    case MAPVK_VSC_TO_VK_EX:
        switch(uCode) {
            case 0x1D: 
                return VK_RCONTROL;
                
            case 0x38: 
                return VK_RMENU;
                
            case 0x2A: 
                return VK_LSHIFT;
                
            case 0x36:  
                return VK_RSHIFT;
                
            case 0x5B: 
                return VK_LWIN;
                
            case 0x5C: 
                return VK_RWIN;
                
            case 0x5D:  
                return VK_APPS;
        default:
            return MapVirtualKeyA(uCode, MAPVK_VSC_TO_VK);
    case MAPVK_VK_TO_VSC_EX:
            //todo
            return MapVirtualKeyA(uCode, MAPVK_VK_TO_VSC);
    }
    default:
        return -1;
    }
}