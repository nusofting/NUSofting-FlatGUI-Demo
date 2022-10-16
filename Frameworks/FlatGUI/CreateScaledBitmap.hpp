#pragma once

CBitmap* createScaledBitmap(const char* fileName = 0, const CCoord  scale = 1.0f, bool reverse = false)
	{
		if(fileName)
		{
			std::string absolutePicPath = Platform::getPathForAppResource(fileName);
			IPlatformBitmap* tempBmp = IPlatformBitmap::createFromPath (absolutePicPath.c_str());
			CBitmap* locBmp =  new CBitmap(tempBmp);
			if(tempBmp && locBmp)
			{
				if(reverse)
				{					
					IPlatformBitmapPixelAccess* cBMPpx = tempBmp->lockPixels(false);
					if(cBMPpx)
					{
						// Get the address of the first line.
						uint8_t* pPxLine = cBMPpx->getAddress(); // temporary buffer
						int32_t iPxStride = cBMPpx->getBytesPerRow(); // iPxStride/4 = tile->getWidth() // colours kBGRA
						const int pxfrmt = cBMPpx->getPixelFormat();
						const int32_t bytes = iPxStride * int(locBmp->getHeight());
						for (int32_t i = 0; i <= bytes - 4; i += 4) // all pxs
						{	
							switch(pxfrmt)
							{
							case IPlatformBitmapPixelAccess::kARGB:
							case IPlatformBitmapPixelAccess::kABGR:
								{
									pPxLine[i]	 =  pPxLine[i];   // A
									pPxLine[i+1] = ~pPxLine[i+1]; 
									pPxLine[i+2] = ~pPxLine[i+2]; 
									pPxLine[i+3] = ~pPxLine[i+3]; 
								} break;	
							case IPlatformBitmapPixelAccess::kRGBA:
							case IPlatformBitmapPixelAccess::kBGRA:
							default:
								{
									pPxLine[i]	 = ~pPxLine[i];   // B
									pPxLine[i+1] = ~pPxLine[i+1]; // G
									pPxLine[i+2] = ~pPxLine[i+2]; // R
									pPxLine[i+3] = pPxLine[i+3];  // A
								} 
							}
						}	
						cBMPpx->forget(); // Unlock the bits. // temporary buffer copied back to bitmap
						tempBmp->setScaleFactor(scale); 
						return locBmp;
					}				
				}
				else // not reverse
				{	
					tempBmp->setScaleFactor(scale); 
					return locBmp;	
				}
			}
		}
        return 0;
	}
