#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include "meshconvert.h"

using namespace std;
using std::string;

//////////////////// logging mumbo - jumbo
char* LOG_LEVEL_STRINGS[4] = {"FATAL","ERROR","INFO ","DEBUG"};

#define LOG_DEBUG 3
#define LOG_INFO 2
#define LOG_ERROR 1
#define LOG_FATAL 0

long loglevel = LOG_DEBUG;

void log(long level, char *fmt, ...) {
	if (level > loglevel)
		return;
	
	// to filter out those which do not fin in the table of strings describing level
	if (level > 3)
		level = 3;
	if (level < 0)
		level = 0;
	
	va_list argptr;
	char result[255];
	
	va_start(argptr, fmt);
	vsnprintf(result, 255, fmt, argptr);
	va_end(argptr);
	
	printf("Log (%s) : %s\n", LOG_LEVEL_STRINGS[level], result);
}

#define log_fatal(...) log(LOG_ERROR, __VA_ARGS__)
#define log_error(...) log(LOG_ERROR, __VA_ARGS__)
#define log_info(...) log(LOG_INFO, __VA_ARGS__)
#define log_debug(...) log(LOG_DEBUG, __VA_ARGS__)

////////////////////// global data - yuck

UVMap *uvs = NULL;
long num_uvs; // TODO: == hdr.num_verts?

VHotObj *vhots = NULL;
Vertex *vertices = NULL;
MeshMaterial *materials = NULL;
MeshMaterialExtra *materialsExtra = NULL;
SubObjectHeader *objects = NULL;

// HERE come the resulting structures we are desperately trying to fill:
// vector<Vertex> out_vertices;


/////////////////////////////////////////// HELPER FUNCTIONS
/* Processes object list, and should prepare the global vertex buffer, index buffer, skeleton tree, vertex - joint mapping
*/
void ProcessObjects(ifstream &in, BinHeadType &thdr, BinHeader &hdr, BinHeader2 &hdr2) {
	objects = new SubObjectHeader[hdr.num_objs];
	
	in.seekg(hdr.offset_objs, ios::beg);
	in.read((char *) objects, sizeof(SubObjectHeader) * hdr.num_objs);
	
	// TODO: I do not have other choice, than to make the skeleton so the VHOTS in the sub-objects are made as bones from the joint point to the vhot.
	// This is because the ogre does not have anything like the VHOT vertices... (This should not be a serious problem, I hope - they only should have compatible numbering	)
	
	// logging the object names:
	for (int x = 0; x < hdr.num_objs; x++) {
		log_debug("       - sub-object name : %s", objects[x].name);
		log_debug("       	- movement type : %d", (int) objects[x].movement);
		log_debug("       	- movement type : %d", (int) objects[x].movement);
	}
	
	// now the important stuff...

	// the object tree goes in such way, that it should be possible to do only one transform per sub-object we process...
	// it seems we have to do transforms. otherwise we would end up having a broken geometry relations
}


/////////////////////////////////////////// Global processing functions
// The object model type of mesh
void readObjectModel(ifstream &in, BinHeadType &thdr) {
	BinHeader hdr;
	BinHeader2 hdr2;
	
	log_info("LGMD mesh processing - e.g. an object");
	long size = 0;
	
	log_debug("Header version : %d", thdr.version);
	
	// determine the header size depending on the version of the mesh
	switch (thdr.version) {
		case 3: size = SIZE_BIN_HDR_V3; break;
		case 4: size = SIZE_BIN_HDR_V4; break;
		case 6: size = SIZE_BIN_HDR_V6; break;
		default: size = -1; // TODO: fixfix
	}
	
	if (size <= 0) {
		log_fatal("FATAL: The object mesh has an unsupported version : %d", thdr.version);
		return;
	}
	
	// erase the header
	memset((char *) &hdr, 0, sizeof(BinHeader));
	
	// read the model header
	in.read((char *) &hdr, size);
	
	// we should definetaly complete the header data depending on the version somehow...
	
	if (thdr.version == 6) { // we have another header to look at
		log_debug("V6 header");
		in.seekg(hdr.offset_hdr2, ios::beg);
		in.read((char *) &hdr2, sizeof(BinHeader2));
		
		// is this a right approach? I hope so
		hdr.offset_uv = hdr2.offset_uv;
		hdr.offset_verts = hdr2.offset_verts;
		hdr.offset_norms = hdr2.offset_norms;
		num_uvs = hdr2.num_uvs;
	} else {
		// It seems that pre-6 version headers do not contain number of uvs
		num_uvs = (hdr.offset_vhots - hdr.offset_uv) / sizeof (UVMap);
		
		log_debug("pre - V6 header, calculated num_uvs = %d", num_uvs);
	}
	
	log_info("Reading Tables:");

	log_info(" * Materials (%d)", hdr.num_mats);
	log_debug("  - offset %06lX",hdr.offset_mats);

	// Materials
	materials = new MeshMaterial[hdr.num_mats];

	// this is not good! we read right through other tables...
	in.seekg(hdr.offset_mats, ios::beg);
	
	in.read((char *) materials, hdr.num_mats * sizeof(MeshMaterial));
	
	// logging the material names:
	for (int x = 0; x < hdr.num_mats; x++)
		log_debug("       - material name : %s", materials[x].name);
	
	// if we need extended attributes
	if ( hdr.mat_flags & MD_MAT_TRANS || hdr.mat_flags & MD_MAT_ILLUM ) {
		log_info(" * Extra materials (%d)", hdr.num_mats);
		log_debug("  - actual offset %06lX", (int) in.tellg());
		// Extra Materials
		materialsExtra = new MeshMaterialExtra[hdr.num_mats];
		in.read((char *) materialsExtra, hdr.num_mats * sizeof(MeshMaterialExtra));
		
	}
	
	// Read the UV map vectors
	log_info(" * UVMAP (%d / %d)", hdr.num_verts, num_uvs);
	log_debug("  - offset %06lX",hdr.offset_uv);
	// prepare and read uvmaps
	uvs = new UVMap[num_uvs];
	
	in.seekg(hdr.offset_uv, ios::beg);
	// I have to rely on shadowspawn here: read uvs in the number of verts - those are differing, but I guess that this is solvable later on
	// After looking into the binary, the num_uvs calculated seem to be reasonable
	// in.read((char *) uvs, hdr.num_verts * sizeof(UVMap));
	in.read((char *) uvs, num_uvs * sizeof(UVMap));
	
	// TODO: shadowspawn reverses the U part of pre-6 version mesh UV table here. See if we need that too


	// VHOT:
	log_info(" * VHOT (%d)", hdr.num_vhots);
	log_debug("  - offset %06lX",hdr.offset_vhots);
	// prepare and read the vhots
	vhots = new VHotObj[hdr.num_vhots];
	in.seekg(hdr.offset_vhots, ios::beg);
	in.read((char *) vhots, hdr.num_vhots * sizeof(VHotObj));
	
	// Vertex table....
	log_info(" * Vertices (%d)", hdr.num_verts);
	log_debug("  - offset %06lX",hdr.offset_verts);
	
	// prepare and read the vertices
	vertices = new Vertex[hdr.num_verts];
	in.seekg(hdr.offset_verts, ios::beg);
	in.read((char *) vertices, hdr.num_verts * sizeof(Vertex));
	
	log_info("Object tree processing:");
	
	ProcessObjects(in, thdr, hdr, hdr2);
	
	// cleanout
	log_info("Releasing used pointers");
	if (vhots != NULL)
		delete[] vhots;
	
	if (vertices != NULL)
		delete[] vertices;
	
	if (uvs != NULL)
		delete[] uvs;
	
	if (materials != NULL)
		delete[] materials;
	
	if (materialsExtra != NULL)
		delete[] materialsExtra;
	
	// the end
	log_info("all done");
}

int main(int argc, char* argv[]) {
	cout << "MeshConvert (.bin to .xml converter for all kinds of LG meshes)\n";


	// open the input bin mesh file
	BinHeadType header;
	
	ifstream input("door17sw.bin", ios::binary); 
	
	input.read((char *) &header, sizeof(header));
	
	
	if (strncmp(header.ID,"LGMD",4) == 0) {
		readObjectModel(input, header);
		
	} else 
	if (strncmp(header.ID,"LGMM",4) == 0) {
		cout << "AI mesh type - Unimplemented for now\n";
	}
		else printf("Unknown type");
	

	input.close();		
	return 0;
}
