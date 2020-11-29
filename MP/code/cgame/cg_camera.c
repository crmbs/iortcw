#include "cg_camera.h"
#include "cg_local.h"
#include "cg_q3mme_demos_camera.h"  // CAM_ORIGIN, CAM_ANGLES, CAM_FOV

void CG_CameraResetInternalLengths (void)
{
	cameraPoint_t *p;

	p = cg.cameraPointsPointer;

	while (p) {
		p->len = -1;
		p = p->next;
	}
}

// from q3mme
static void wolfcamCameraPointMatch (const cameraPoint_t *point, int mask, const cameraPoint_t *match[4])
{
	const cameraPoint_t *p;

	p = point;
	while (p && !(p->flags & mask))
		p = p->prev;

	if (!p) {
		match[0] = 0;
		match[1] = 0;
		p = point;
		while (p && !(p->flags & mask))
			p = p->next;
		if (!p) {
			match[2] = 0;
			match[3] = 0;
			return;
		}
	} else {
		match[1] = p;
		p = p->prev;

		while (p && !(p->flags & mask))
			p = p->prev;

		match[0] = p;
	}
	p = point->next;
	while (p && !(p->flags & mask))
		p = p->next;
	match[2] = p;
	if (p)
		p = p->next;
	while (p && !(p->flags & mask))
		p = p->next;
	match[3] = p;
}

// from q3mme
static qboolean wolfcamCameraMatchOrigin (const cameraPoint_t *match[4], vec3_t origin[4])
{
	if ( !match[1] )
		return qfalse;

	VectorCopy( match[1]->origin, origin[1] );

	if (match[0])
		VectorCopy( match[0]->origin, origin[0] );
	else if (match[2])
		VectorSubDelta( match[1]->origin, match[2]->origin, origin[0] );
	else
		VectorCopy( match[1]->origin, origin[0] );

	if (match[2]) {
		VectorCopy( match[2]->origin, origin[2] );
		if (match[3])
			VectorCopy( match[3]->origin, origin[3] );
		else
			VectorAddDelta( match[1]->origin, match[2]->origin, origin[3] );
	} else if ( match[0] ) {
		VectorAddDelta( match[0]->origin, match[1]->origin, origin[2] );
		VectorAddDelta( match[0]->origin, match[1]->origin, origin[3] );
	} else {
		VectorCopy( match[1]->origin, origin[2] );
		VectorCopy( match[1]->origin, origin[3] );
	}
	return qtrue;
}

// from q3mme
static void wolfcamCameraMatchAt (double ftime, int mask, const cameraPoint_t *match[4])
{
	const cameraPoint_t *p;

	p = cg.cameraPointsPointer;

	match[0] = match[1] = 0;
	while( p ) {
		if (p->cgtime > ftime)
			break;
		if (p->flags & mask) {
			match[0] = match[1];
			match[1] = p;
		}
		p = p->next;
	}

	while (p && !(p->flags & mask))
		p = p->next;

	match[2] = p;
	if (p)
		p = p->next;

	while (p && !(p->flags & mask))
		p = p->next;

	match[3] = p;
}

// from q3mme
static float wolfcamCameraPointLength (posInterpolate_t posType, cameraPoint_t *point)
{
	int i;
	cameraPoint_t *match[4];
	vec3_t control[4];
	vec3_t lastOrigin, nextOrigin;
	float len = 0, step = 0, addStep, distance, deltaDist = 0.01f;

	if (point->len >= 0) {
		return point->len;
	}

	wolfcamCameraPointMatch(point, CAM_ORIGIN, (const cameraPoint_t **)match);


	if (!wolfcamCameraMatchOrigin( (const cameraPoint_t **)match, control )) {
		point->len = 0;
		return 0;
	}

#if 0  //FIXME linear
	if ( demo.camera.smoothPos == posLinear ) {
		point->len = VectorDistance( control[1], control[2] );
		return point->len;
	}
#endif
	if (posType == posLinear) {
		Com_Printf("^1wolfcamCameraPointLength pos is linear\n");
		posType = posBezier;
	}

	posGet( 0, posType, (const vec3_t *)control, lastOrigin );
	if (cg_q3mmeCameraSmoothPos.value >= 1)
		deltaDist = 0.01f / cg_q3mmeCameraSmoothPos.value;
	while (step < 1) {
		addStep = 1 - step;
		if (addStep > 0.01f)
			addStep = 0.01f;
		for (i = 0; i < 10; i++) {
			posGet( step+addStep, posType, (const vec3_t *)control, nextOrigin );
			distance = VectorDistanceSquared( lastOrigin, nextOrigin);
			if ( distance <= deltaDist)
				break;
			addStep *= 0.7f;
		}
		step += addStep;
		len += sqrt( distance );
		VectorCopy( nextOrigin, lastOrigin );
		if (!distance)
			break;
	}
	point->len = len;
	return point->len;
}

// from q3mme
qboolean CG_CameraSplineOriginAt (double ftime, posInterpolate_t posType, vec3_t origin)
{
	cameraPoint_t *match[4];
	float searchLen;
	vec3_t dx, dy;
	vec3_t control[4];
	vec3_t nextOrigin;
	float len = 0, step = 0, addStep, distance, deltaDist = 0.01f;
	int	i;

	//int debugCount;
	static float LastLen = 0;
	static float LastStep = 0;
	static double LastFtime = 0;
	static vec3_t LastControl[4];
	static float LastSearchLen;
	static vec3_t LastOrigin;
	static posInterpolate_t LastPosType;
	static float LastQ3mmeSmoothPos;
	static qboolean LastValuesSet = qfalse;

	wolfcamCameraMatchAt(ftime, CAM_ORIGIN, (const cameraPoint_t **)match);

	if (!match[1]) {
		if (!match[2])
			return qfalse;
		if (!match[3])
			return qfalse;
		wolfcamCameraPointMatch(match[2], CAM_ORIGIN, (const cameraPoint_t **)match );
		searchLen = 0;
	} else if (!match[2]) {
		/* Only match[1] is valid, will be copied to all points */
		searchLen = 0;
	} else {
		dx[1] = match[2]->cgtime - match[1]->cgtime;
		dy[1] = wolfcamCameraPointLength(posType, match[1]);
		if (match[0]) {
			dx[0] = match[1]->cgtime - match[0]->cgtime;
			dy[0] = wolfcamCameraPointLength(posType, match[0]);
		} else {
			dx[0] = dx[1];
			dy[0] = dy[1];
		}
		if (match[3]) {
			dx[2] = match[3]->cgtime - match[2]->cgtime;
			dy[2] = wolfcamCameraPointLength(posType, match[2]);
		} else {
			dx[2] = dx[1];
			dy[2] = dy[1];
		}
		searchLen = dsplineCalc( (ftime - match[1]->cgtime), dx, dy, 0 );
	}

	if (!wolfcamCameraMatchOrigin( (const cameraPoint_t **)match, control ))
		return qfalse;

#if 0  //FIXME linear
	//if ( demo.camera.smoothPos == posLinear ) {
	if (posType == posLinear) {
		float t = searchLen / match[1]->len ;
		posGet( t, posType, (const vec3_t *)control, origin );
		return qtrue;
	}
#endif
	if (posType == posLinear) {
		Com_Printf("^1ERROR:  posLinear specified\n");
		posType = posBezier;
	}

	//Com_Printf("calc: %f -> %f (%f)\n", match[1] != NULL ? match[1]->cgtime : -1.0f, ftime, match[1] != NULL ? ftime - match[1]->cgtime : -1.0f);
	//debugCount = 0;

	// optimization to avoid starting from step == 0 every time
	if (LastValuesSet  &&
		VectorCompare(control[0], LastControl[0])  &&  VectorCompare(control[1], LastControl[1])  &&  VectorCompare(control[2], LastControl[2])  &&  VectorCompare(control[3], LastControl[3])  &&
		ftime >= LastFtime  &&
		searchLen > LastSearchLen  &&
		LastPosType == posType  &&
		cg_q3mmeCameraSmoothPos.value == LastQ3mmeSmoothPos) {

		step = LastStep;
		len = LastLen;
		VectorCopy(LastOrigin, origin);
		//Com_Printf("step %f  len %f\n", step, len);
	} else {
		step = 0;
		len = 0;
		posGet( 0, posType, (const vec3_t *)control, origin );
	}

	if (cg_q3mmeCameraSmoothPos.value >= 1) {
		deltaDist = 0.01f / cg_q3mmeCameraSmoothPos.value;
	}

	// loop can be called thousands of times
	while (step < 1) {
		LastStep = step;
		LastLen = len;
		VectorCopy(origin, LastOrigin);

		addStep = 1 - step;
		if (addStep > 0.01f)
			addStep = 0.01f;
		for (i = 0; i < 10; i++) {
			posGet( step+addStep, posType, (const vec3_t *)control, nextOrigin );
			distance = VectorDistanceSquared( origin, nextOrigin);
			if ( distance <= deltaDist)
				break;
			addStep *= 0.7f;
		}
		distance = sqrt( distance );
		if (len + distance > searchLen) {
			break;
		}

		len += distance;
		step += addStep;
		VectorCopy( nextOrigin, origin );
		//Com_Printf("nextorigin:  %f %f %f\n", nextOrigin[0], nextOrigin[1], nextOrigin[2]);
		//Com_Printf("addStep:  %f\n", addStep);
		//debugCount++;
	}

	LastFtime = ftime;
	for (i = 0;  i < 4;  i++) {
		VectorCopy(control[i], LastControl[i]);
	}
	LastSearchLen = searchLen;
	LastPosType = posType;
	LastQ3mmeSmoothPos = cg_q3mmeCameraSmoothPos.value;
	LastValuesSet = qtrue;

	//Com_Printf("    debugCount: %d\n", debugCount);
	return qtrue;
}

// from q3mme
qboolean CG_CameraSplineAnglesAt (double ftime, vec3_t angles)
{
    cameraPoint_t *match[4];
	float lerp;
	vec3_t tempAngles;
	Quat_t q0, q1, q2, q3, qr;
    //double timeFraction;

    wolfcamCameraMatchAt(ftime, CAM_ANGLES, (const cameraPoint_t **)match);

	if (!match[1]) {
		if (match[2]) {
			VectorCopy( match[2]->angles, angles );
			return qtrue;
		}
		return qfalse;
	}
	if (!match[2]) {
		VectorCopy( match[1]->angles, angles );
		return qtrue;
	}

    //timeFraction = ftime - match[1]->cgtime;
	lerp = ((ftime - match[1]->cgtime) ) / (match[2]->cgtime - match[1]->cgtime);

    //Com_Printf("lerp:  %f\n", lerp);

    //	switch ( demo.camera.smoothAngles ) {
	//case angleLinear:
	//	LerpAngles( match[1]->angles, match[2]->angles, angles, lerp );
	//	break;
	//default:
	//case angleQuat:
		QuatFromAngles( match[1]->angles, q1 );
		if ( match[0] ) {
			QuatFromAnglesClosest( match[0]->angles, q1, q0 );
		} else {
			VectorSubDelta( match[1]->angles, match[2]->angles, tempAngles );
			QuatFromAnglesClosest( tempAngles, q1, q0 );
		}
		QuatFromAnglesClosest( match[2]->angles, q1, q2 );
		if (match[3]) {
			QuatFromAnglesClosest( match[3]->angles, q2, q3 );
		} else {
			VectorAddDelta( match[1]->angles, match[2]->angles, tempAngles );
			QuatFromAnglesClosest( tempAngles, q2, q3 );
		}
		QuatSquad( lerp, q0, q1, q2, q3, qr );
		QuatToAngles( qr, angles );
        //}
	return qtrue;
}

// from q3mme
qboolean CG_CameraSplineFovAt (double ftime, float *fov)
{
	cameraPoint_t	*match[4];
	float				lerp;
	float				f[4];
	int					t[4];

	wolfcamCameraMatchAt( ftime, CAM_FOV, (const cameraPoint_t **)match );

	if (!match[1]) {
		if (match[2]) {
			*fov = match[2]->fov;
			return qtrue;
		}
		return qfalse;
	}
	if (!match[2]) {
		*fov = match[1]->fov;
		return qtrue;
	}

	f[1] = match[1]->fov;
	t[1] = match[1]->cgtime;
	f[2] = match[2]->fov;
	t[2] = match[2]->cgtime;

	if ( match[0] ) {
		f[0] = match[0]->fov;
		t[0] = match[0]->cgtime;
	} else {
		f[0] = f[1] - (f[2] - f[1]);
		t[0] = t[1] - (t[2] - t[1]);
	}
	if (match[3]) {
		f[3] = match[3]->fov;
		t[3] = match[3]->cgtime;
	} else {
		f[3] = f[2] + (f[2] - f[1]);
		t[3] = t[2] + (t[2] - t[1]);
	}
	lerp = (ftime - t[1] ) / (t[2] - t[1]);
	VectorTimeSpline( lerp, t, (float*)f, fov, 1 );
	return qtrue;
}

// longest distance for either yaw or pitch
float CG_CameraAngleLongestDistanceNoRoll (const vec3_t a0, const vec3_t a1)
{
	float dist;
	float d0, d1;

	d0 = fabs(AngleSubtract(a0[YAW], a1[YAW]));
	d1 = fabs(AngleSubtract(a0[PITCH], a1[PITCH]));

	if (d0 > d1) {
		dist = d0;
	} else {
		dist = d1;
	}

	return dist;
}

// longest distance for yaw, pitch, or roll
float CG_CameraAngleLongestDistanceWithRoll (const vec3_t a0, const vec3_t a1)
{
	float dist;
	float d0, d1, d2;

	d0 = fabs(AngleSubtract(a0[YAW], a1[YAW]));
	d1 = fabs(AngleSubtract(a0[PITCH], a1[PITCH]));
	d2 = fabs(AngleSubtract(a0[ROLL], a1[ROLL]));

	if (d0 > d1) {
		dist = d0;
	} else {
		dist = d1;
	}

	if (d2 > dist) {
		dist = d2;
	}

	return dist;
}
