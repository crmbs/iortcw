
//FIXME hack, older jpeg-6b could accept rgba input buffer, now it has to be rgb
static void convert_rgba_to_rgb (byte *buffer, int width, int height)
{
	byte *src;
	byte *dst;
	int totalSize;


	totalSize = width * height * 4;
	src = buffer;
	dst = buffer;
	while (src < (buffer + totalSize)) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		// skip alpha src[3]

		src += 4;
		dst += 3;
	}
}

// swap rgb to bgr
static void swap_bgr (byte *buffer, int width, int height, qboolean hasAlpha)
{
	int temp;
	int i;
	int c;
	int psize;

	psize = 3 + (hasAlpha ? 1 : 0);

	c = width * height * psize;
	for (i = 0;  i < c;  i += psize) {
		temp = buffer[i];
		buffer[i] = buffer[i + 2];
		buffer[i + 2] = temp;
	}
}

const void *RB_TakeVideoFrameCmd (const void *data, shotData_t *shotData)
{
	const videoFrameCommand_t	*cmd;
	int												frameSize;
	int												i;
	char finalName[MAX_QPATH];
	qboolean fetchBufferHasAlpha = qfalse;
	qboolean fetchBufferHasRGB = qtrue;   // does buffer have RGB or BGR
	int glMode = GL_RGB;
	char *sbuf;
	__m64 *outAlign = NULL;
	byte *fetchBuffer;
	int blurFrames;
	int blurOverlap;
	qboolean useBlur;
	int frameRateDivider;

	cmd = (const videoFrameCommand_t *)data;

	useBlur = qfalse;
	blurFrames = ri.Cvar_VariableIntegerValue("mme_blurFrames");
	if (blurFrames == 0  ||  blurFrames == 1) {
		useBlur = qfalse;
	} else {
		useBlur = qtrue;
	}
	blurOverlap = ri.Cvar_VariableIntegerValue("mme_blurOverlap");
	if (blurOverlap > 0) {
		useBlur = qtrue;
	}
	frameRateDivider = ri.Cvar_VariableIntegerValue("cl_aviFrameRateDivider");
	if (frameRateDivider < 1) {
		frameRateDivider = 1;
	}

	shotData->pixelCount = cmd->width * cmd->height;

	if (cmd->png) {
		fetchBufferHasAlpha = qtrue;
		fetchBufferHasRGB = qtrue;
		glMode = GL_RGBA;
		fetchBuffer = cmd->captureBuffer + 18;
		fetchBuffer = (byte *)(((uintptr_t)fetchBuffer + 15) & ~15);
		fetchBuffer -= 18;
	} else {  //  not png
		sbuf = finalName;
		ri.Cvar_VariableStringBuffer("cl_aviFetchMode", sbuf, MAX_QPATH);
		if (!Q_stricmp("gl_rgba", sbuf)) {
			fetchBufferHasAlpha = qtrue;
			fetchBufferHasRGB = qtrue;
			glMode = GL_RGBA;
		} else if (!Q_stricmp("gl_rgb", sbuf)) {
			fetchBufferHasAlpha = qfalse;
			fetchBufferHasRGB = qtrue;
			glMode = GL_RGB;
		} else if (!Q_stricmp("gl_bgr", sbuf)) {
			fetchBufferHasAlpha = qfalse;
			fetchBufferHasRGB = qfalse;
			glMode = GL_BGR;
		} else if (!Q_stricmp("gl_bgra", sbuf)) {
			fetchBufferHasAlpha = qtrue;
			fetchBufferHasRGB = qfalse;
			glMode = GL_BGRA;
		} else {
			ri.Printf(PRINT_ALL, "unknown glmode using GL_RGB\n");
			fetchBufferHasAlpha = qfalse;
			fetchBufferHasRGB = qtrue;
			glMode = GL_RGB;
		}

		if (cmd->jpg  ||  (cmd->avi  &&  cmd->motionJpeg)) {
			//FIXME jpg check not needed anymore
			fetchBuffer = cmd->captureBuffer + 18;
			fetchBuffer = (byte *)(((uintptr_t)fetchBuffer + 15) & ~15);
			fetchBuffer -= 18;
		} else {
			fetchBuffer = cmd->encodeBuffer + 18;
			fetchBuffer = (byte *)(((uintptr_t)fetchBuffer + 15) & ~15);
			fetchBuffer -= 18;
		}
	}

	//FIXME why is this needed?
	if (useBlur) {
		glMode = GL_BGR;
		fetchBufferHasAlpha = qfalse;
		fetchBufferHasRGB = qfalse;
	}

	if (!useBlur) {
		//ri.Printf(PRINT_ALL, "no blur pic count: %d\n", cmd->picCount + 1);
		if ((cmd->picCount + 1) % frameRateDivider != 0) {
			//ri.Printf(PRINT_ALL, "    skipping %d\n", cmd->picCount + 1);
			goto dontwrite;
		}
		//ri.Printf(PRINT_ALL, "writing %d\n", cmd->picCount + 1);
		qglReadPixels(0, 0, cmd->width, cmd->height, glMode, GL_UNSIGNED_BYTE, fetchBuffer + 18);
		//R_MME_GetMultiShot(fetchBuffer + 18, cmd->width, cmd->height, glMode);
		R_GammaCorrect(fetchBuffer + 18, cmd->width * cmd->height * (3 + fetchBufferHasAlpha));
	} else {  // use blur

		if (shotData->allocFailed) {
			ri.Printf(PRINT_ALL, "^1ERROR: not capturing because blur allocation failed\n");
		}

		if (shotData->control.totalFrames && !shotData->allocFailed) {
			//mmeBlurBlock_t *blurShot = &blurData.shot;
			//mmeBlurBlock_t *blurDepth = &blurData.depth;
			//mmeBlurBlock_t *blurStencil = &blurData.stencil;

			/* Test if we blur with overlapping frames */
			if ( shotData->control.overlapFrames ) {
				/* First frame in a sequence, fill the buffer with the last frames */
				if (shotData->control.totalIndex == 0) {
					int i;
					for (i = 0; i < shotData->control.overlapFrames; i++) {
						R_MME_BlurOverlapAdd(&shotData->shot, i);

						//FIXME implement
						shotData->control.totalIndex++;
					}
				}

				if (1) {  // ( mme_saveShot->integer == 1 ) {
					byte *shotBuf = R_MME_BlurOverlapBuf(&shotData->shot);

					qglReadPixels(0, 0, cmd->width, cmd->height, glMode, GL_UNSIGNED_BYTE, shotBuf);
					//R_MME_GetMultiShot(shotBuf, cmd->width, cmd->height, glMode);

					outAlign = (__m64 *)(fetchBuffer + 18);
					R_GammaCorrect(shotBuf, cmd->width * cmd->height * (3 + fetchBufferHasAlpha));

					R_MME_BlurOverlapAdd(&shotData->shot, 0);
				}

				shotData->control.overlapIndex++;
				shotData->control.totalIndex++;
			} else {  // shotData->overlapFrames, just blur no overlap
				qglReadPixels(0, 0, cmd->width, cmd->height, glMode, GL_UNSIGNED_BYTE, fetchBuffer + 18);
				//R_MME_GetMultiShot(fetchBuffer + 18, cmd->width, cmd->height, glMode);
				R_GammaCorrect(fetchBuffer + 18, cmd->width * cmd->height * (3 + fetchBufferHasAlpha));

				outAlign = (__m64 *)(fetchBuffer + 18);

				R_MME_BlurAccumAdd(&shotData->shot, outAlign);

				shotData->control.totalIndex++;

			}  // shotData->overlapTotal

			if (shotData->control.totalIndex >= shotData->control.totalFrames) {
				shotData->control.totalIndex = 0;
				R_MME_BlurAccumShift(&shotData->shot);

				outAlign = shotData->shot.accum;

				//ri.Printf(PRINT_ALL, "pic count: %d\n", cmd->picCount);
				if (((cmd->picCount + 1) * blurFrames) % frameRateDivider != 0) {
					goto dontwrite;
				}
			} else {
				// skip saving the shot
				//goto done;
				goto dontwrite;
			}

			memcpy(fetchBuffer + 18, shotData->shot.accum, shotData->pixelCount * 3);
		} else {
			// shouldn't happen
		}
	}

	if (cmd->tga) {
		byte *buffer;
		int width, height;
		int c;
		int count;

		if (blurFrames > 1) {
			count = cmd->picCount / blurFrames;
		} else {
			count = cmd->picCount;
		}
		count /= frameRateDivider;

		//FIXME hack
		if (shotData == &shotDataLeft) {
			Com_sprintf(finalName, MAX_QPATH, "videos/%s-left-%010d.tga", cmd->givenFileName, count);
		} else {
			Com_sprintf(finalName, MAX_QPATH, "videos/%s-%010d.tga", cmd->givenFileName, count);
		}
		width = cmd->width;
		height = cmd->height;
		buffer = fetchBuffer;
		Com_Memset (buffer, 0, 18);
        buffer[2] = 2;          // uncompressed type
        buffer[12] = width & 255;
        buffer[13] = width >> 8;
        buffer[14] = height & 255;
        buffer[15] = height >> 8;
        buffer[16] = 24 + 8 * fetchBufferHasAlpha;        // pixel size

		frameSize = cmd->width * cmd->height;

		if (fetchBufferHasRGB) {
			swap_bgr(buffer + 18, width, height, fetchBufferHasAlpha);
		}

		ri.FS_WriteFile(finalName, buffer, width * height * (3 + fetchBufferHasAlpha) + 18);

		goto done;
	}

	if (cmd->jpg  ||  cmd->png) {
		byte *buffer;
		int width, height;
		int count;
		const char *type = "png";

		if (cmd->jpg) {
			type = "jpg";
		}

		if (blurFrames > 1) {
			count = cmd->picCount / blurFrames;
		} else {
			count = cmd->picCount;
		}
		count /= frameRateDivider;

		//FIXME hack
		if (shotData == &shotDataLeft) {
			Com_sprintf(finalName, MAX_QPATH, "videos/%s-left-%010d.%s", cmd->givenFileName, count, type);
		} else {
			Com_sprintf(finalName, MAX_QPATH, "videos/%s-%010d.%s", cmd->givenFileName, count, type);
		}

		width = cmd->width;
		height = cmd->height;
		buffer = fetchBuffer + 18;
		ri.FS_WriteFile(finalName, buffer, 1);  // create path
		if (cmd->jpg) {
			if (!fetchBufferHasRGB) {
				swap_bgr(buffer, width, height, fetchBufferHasAlpha);
			}

			if (fetchBufferHasAlpha) {
				convert_rgba_to_rgb(buffer, width, height);
			}

			RE_SaveJPG(finalName, r_jpegCompressionQuality->integer, width, height, buffer, 0);
		} else {  // png
			if (!fetchBufferHasRGB) {
				swap_bgr(buffer, width, height, fetchBufferHasAlpha);
			}

			SavePNG(finalName, buffer, width, height, (3 + fetchBufferHasAlpha));
		}

		goto done;
	}

	if( cmd->avi  &&  cmd->motionJpeg )
	{
		if (!fetchBufferHasRGB) {
			swap_bgr(fetchBuffer + 18, cmd->width, cmd->height, fetchBufferHasAlpha);
		}

		if (fetchBufferHasAlpha) {
			convert_rgba_to_rgb(fetchBuffer + 18, cmd->width, cmd->height);
		}

		frameSize = RE_SaveJPGToBuffer(cmd->encodeBuffer + 18, /*FIXME*/ cmd->width * cmd->height * 3, r_jpegCompressionQuality->integer, cmd->width, cmd->height, fetchBuffer + 18, 0);
		ri.CL_WriteAVIVideoFrame(ri.afdMain, cmd->encodeBuffer + 18, frameSize);

	} else if (cmd->avi) {
		frameSize = cmd->width * cmd->height;
		//byte *buffer;
		int c;
		byte *outBuffer;

		//buffer = cmd->encodeBuffer;
		//outBuffer = cmd->encodeBuffer;
		outBuffer = fetchBuffer;
		if (fetchBufferHasAlpha  &&  fetchBufferHasRGB) {
			// gl_rgba
			outBuffer = cmd->captureBuffer;
			//frameSize = cmd->width * cmd->height;
			for (i = 0;  i < frameSize;  i++) {
				outBuffer[i * 3 + 0 + 18] = fetchBuffer[i * 4 + 2 + 18];
				outBuffer[i * 3 + 1 + 18] = fetchBuffer[i * 4 + 1 + 18];
				outBuffer[i * 3 + 2 + 18] = fetchBuffer[i * 4 + 0 + 18];
			}
		} else if (fetchBufferHasAlpha  &&  !fetchBufferHasRGB) {
			// gl_bgra
			outBuffer = cmd->captureBuffer;
			//frameSize = cmd->width * cmd->height;
			for (i = 0;  i < frameSize;  i++) {
				outBuffer[i * 3 + 0 + 18] = fetchBuffer[i * 4 + 0 + 18];
				outBuffer[i * 3 + 1 + 18] = fetchBuffer[i * 4 + 1 + 18];
				outBuffer[i * 3 + 2 + 18] = fetchBuffer[i * 4 + 2 + 18];
			}
		} else if (fetchBufferHasRGB) {
			// gl_rbg
			// there is no alpha
			swap_bgr(outBuffer + 18, cmd->width, cmd->height, qfalse);
		} else {
			// it's just gl_bgr, no change needed
		}

		ri.CL_WriteAVIVideoFrame(ri.afdMain, outBuffer + 18, frameSize * 3);
	}

 done:

 dontwrite:
	//ri.Hunk_FreeTempMemory( outAlloc );
	//shotData->shot.take = qfalse;

	return (const void *)(cmd + 1);
}
