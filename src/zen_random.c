/*  Zenroom (DECODE project)
 *
 *  (c) Copyright 2017-2018 Dyne.org foundation
 *  designed, written and maintained by Denis Roio <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <jutils.h>
#include <zen_error.h>
#include <lua_functions.h>

#include <amcl.h>
#include <pbc_support.h>

#include <zenroom.h>
#include <zen_octet.h>
#include <zen_memory.h>
#include <zen_big.h>
#include <randombytes.h>

// easier name (csprng comes from amcl.h in milagro)
#define RNG csprng

RNG* rng_new(lua_State *L) {
	HERE();
    RNG *rng = (RNG*)lua_newuserdata(L, sizeof(csprng));
    if(!rng) {
	    lerror(L, "Error allocating new random number generator in %s",__func__);
	    return NULL; }
    luaL_getmetatable(L, "zenroom.rng");
    lua_setmetatable(L, -2);

	char *tmp = zen_memory_alloc(256);
	randombytes(tmp,252);
	// using time() from milagro
	unsign32 ttmp = GET_TIME();
	tmp[252] = (ttmp >> 24) & 0xff;
	tmp[253] = (ttmp >> 16) & 0xff;
	tmp[254] = (ttmp >>  8) & 0xff;
	tmp[255] =  ttmp & 0xff;
	RAND_seed(rng,256,tmp);
	zen_memory_free(tmp);
	return(rng);
}
	
RNG* rng_arg(lua_State *L, int n) {
	void *ud = luaL_checkudata(L, n, "zenroom.rng");
	luaL_argcheck(L, ud != NULL, n, "rng class expected");	
	return((RNG*)ud);
}

int rng_destroy(lua_State *L) {
	HERE();
	(void)L;
	return 0;
}

static int newrng(lua_State *L) {
	HERE();
    RNG *rng = rng_new(L); SAFE(rng);
    return 1;
}

static int rng_oct(lua_State *L) {
	RNG *rng = rng_arg(L,1); SAFE(rng);
	int tn;
	lua_Number n = lua_tonumberx(L, 2, &tn);
	octet *o = o_new(L,(int)n); SAFE(o);
	OCT_rand(o,rng,(int)n);
	return 1;
}

static int rng_big(lua_State *L) {
	RNG *rng = rng_arg(L,1); SAFE(rng);
	big *res = big_new(L); SAFE(res);
	BIG_random(res->val, rng);
	return(1);
}

/**
   Returns a random BIG number reduced to modulo first argument, removing bias.
   see WCC function randomnum in milagro.
*/
static int rng_modbig(lua_State *L) {
	RNG *rng = rng_arg(L,1); SAFE(rng);
	big *modulus = big_arg(L,2); SAFE(modulus);	
	big *res = big_new(L); SAFE(res);
	BIG_randomnum(res->val,modulus->val,rng);
	return(1);
}

int luaopen_rng(lua_State *L) {
	const struct luaL_Reg rng_class[] = {
		{"new",newrng},
		{NULL,NULL}
	};
	const struct luaL_Reg rng_methods[] = {
		{"octet", rng_oct},
		{"oct", rng_oct},
		{"big", rng_big},
		{"modbig", rng_modbig},
		{"__gc", rng_destroy},
		{NULL,NULL}
	};
	zen_add_class(L, "rng", rng_class, rng_methods);
	return 1;
}
