#include "..\\header\\converter.h"

using namespace types;
using namespace google::protobuf;
using namespace logging;

// Containers and strings
using std::string;
using std::unordered_map;
using std::vector;
using std::priority_queue;
// Logging
using logging::Logger;
// Mathtools stuff
using mathtools::triangle;
using mathtools::vec2;
using mathtools::vec3;

namespace osmconverter
{
	///////////////////////////////////////////////////////
	// Initialization
	///////////////////////////////////////////////////////
	Converter::Converter()
	{
		m_major = VERSION_MAJOR;
		m_minor = VERSION_MINOR;
		m_patch = VERSION_PATCH;

		m_debug = false;
		m_line = true;
		m_overflow = false;
		m_update = false;

		m_read_type[0] = true;
		m_read_type[1] = true;
		m_read_type[2] = true;

		m_lat_step = 0.0;
		m_lon_step = 0.0;
		m_minlat = m_maxlat = 0.0;
		m_minlon = m_maxlon = 0.0;

		SetSorting(Sorting::first_node);

		logger = Logger();

		m_nodes = vector<Node>();
		m_nodes.reserve(100000);
		m_singles = vector<NodeX>();
		m_node_map = unordered_map<long long, size_t>();
		m_node_map.reserve(100000);

		m_ways = vector<Way>();
		m_ways.reserve(100000);
		m_ways_left = vector<WayX>();
		m_way_map = unordered_map<long long, size_t>();
		m_way_map.reserve(100000);
		m_ways_left_map = unordered_map<long long, size_t>();

		m_relations = vector<Relation>();
		m_relations.reserve(10000);
		m_rels_left = vector<RelationX>();
		m_rel_map = unordered_map<long long, size_t>();
		m_rel_map.reserve(10000);
		m_rels_left_map = unordered_map<long long, size_t>();

		m_tiles = vector<Tile>();
	}

	void Converter::CleanUp()
	{
		m_nodes.clear();
		m_node_map.clear();
		m_singles.clear();

		m_ways.clear();
		m_way_map.clear();
		m_ways_left.clear();
		m_ways_left_map.clear();

		m_relations.clear();
		m_rel_map.clear();
		m_rels_left.clear();
		m_rels_left_map.clear();

		m_tiles.clear();
	}

	///////////////////////////////////////////////////////
	// Conversion Parameters and Flags
	///////////////////////////////////////////////////////
	void Converter::SetParameters(string in, string out, bool d, bool l, logging::LogLvl log, size_t lods[16], types::Sorting sort)
	{
		m_input = in;
		m_debug = d;
		m_line = l;

		SetLoDs(lods);
		SetSorting(sort);
		SetLoggingLevel(log);
		SetOutputDirectory(out);

		logger.Log(LogLvl::info, "Converter parametes have been set to:");
		utility::PrintUserInput(m_input, m_output, m_debug, m_line, log, m_lods, m_sort);
	}

	void Converter::SetOutputDirectory(string s)
	{
		// Check if path actually exists
		size_t converted = 0, stringsize = strlen(s.data()) + 1;
		wchar_t *path = new wchar_t[stringsize + OUTDIR_LEN];

		mbstowcs_s(&converted, path, stringsize + OUTDIR_LEN, s.data(), _TRUNCATE);
		wcscat_s(path, stringsize + OUTDIR_LEN, OUTDIR);

		DWORD result = CreateDirectory(path, NULL);

		char *full = new char[stringsize + OUTDIR_LEN];
		size_t err = wcsrtombs_s(&converted, full, stringsize + OUTDIR_LEN, (const wchar_t **)&path, _TRUNCATE, NULL);
		if (err == (size_t)-1)
			throw invalid_argument("Unable to construct multibyte string from wide-character string");

		if (result == 0)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_DIR_NOT_EMPTY)
			{
				logger.Log(LogLvl::info, "Directory already exists, no new directory created");
				m_output.assign(full);
				ClearDirectory();
			}
			else
			{
				logger.Log(LogLvl::warning, "New directory could not be created, current directory used for output.");
				m_output.assign(s);
			}
		}
		else
		{
			m_output.assign(full);
			logger.Log(LogLvl::info, "Created output directory: " + m_output);
		}
	}

	void Converter::ClearDirectory()
	{
		for (short i = C_MAX_LOD; i > C_MIN_LOD; i--)
		{
			string name = GetDataFilename(i);
			size_t converted = 0, stringsize = strlen(name.data()) + 1;
			wchar_t *path = new wchar_t[stringsize];

			mbstowcs_s(&converted, path, stringsize, name.data(), _TRUNCATE);
			// Delete data file if it exists
			if (PathFileExists(path) == TRUE)
			{
				logger.Log(LogLvl::info, 1, string("deleted existing data file for LoD " + std::to_string(i)));
				DeleteFile(path);
			}

			name = GetDataFilename(i) + ".txt";
			stringsize = strlen(name.data()) + 1;
			path = new wchar_t[stringsize];

			mbstowcs_s(&converted, path, stringsize, name.data(), _TRUNCATE);
			// Delete lookup file if it exists
			if (PathFileExists(path) == TRUE)
			{
				logger.Log(LogLvl::info, 1, string("deleted existing data text file for LoD " + std::to_string(i)));
				DeleteFile(path);
			}

			name = GetLookupFilename(i);
			stringsize = strlen(name.data()) + 1;
			path = new wchar_t[stringsize];

			mbstowcs_s(&converted, path, stringsize, name.data(), _TRUNCATE);
			// Delete lookup file if it exists
			if (PathFileExists(path) == TRUE)
			{
				logger.Log(LogLvl::info, 1, string("deleted existing lookup file for LoD " + std::to_string(i)));
				DeleteFile(path);
			}

			name = GetLookupFilename(i) + ".txt";
			stringsize = strlen(name.data()) + 1;
			path = new wchar_t[stringsize];

			mbstowcs_s(&converted, path, stringsize, name.data(), _TRUNCATE);
			// Delete lookup file if it exists
			if (PathFileExists(path) == TRUE)
			{
				logger.Log(LogLvl::info, 1, string("deleted existing lookup text file for LoD " + std::to_string(i)));
				DeleteFile(path);
			}
		}
		string name = m_output + "\\meta";
		size_t converted = 0, stringsize = strlen(name.data()) + 1;
		wchar_t *path = new wchar_t[stringsize];

		mbstowcs_s(&converted, path, stringsize, name.data(), _TRUNCATE);
		// Delete meta file if it exists
		if (PathFileExists(path) == TRUE)
		{
			logger.Log(LogLvl::info, 1, "deleted existing meta file");
			DeleteFile(path);
		}

		name = m_output + "\\meta.txt";
		stringsize = strlen(name.data()) + 1;
		path = new wchar_t[stringsize];

		mbstowcs_s(&converted, path, stringsize, name.data(), _TRUNCATE);
		// Delete lookup file if it exists
		if (PathFileExists(path) == TRUE)
		{
			logger.Log(LogLvl::info, 1, ("deleted existing meta text file"));
			DeleteFile(path);
		}
	}

	void Converter::SetSorting(types::Sorting s)
	{
		m_sort = s;
	}

	void Converter::SetLoDs(size_t lods[16])
	{
		for (int i = C_MIN_LOD; i <= C_MAX_LOD; i++)
		{
			m_lods[i] = lods[i];
		}
	}

	void Converter::SetLoggingLevel(logging::LogLvl lvl)
	{
		logger.SetMaxLoggingLevel(lvl);
	}

	///////////////////////////////////////////////////////
	// Reading Input Data
	///////////////////////////////////////////////////////
	void Converter::ConvertPBF()
	{
		// Data arrays
		char *buffer = new char[MAX_UNCOMPRESSED_BLOB_SIZE];
		char *unpack_buffer = new char[MAX_UNCOMPRESSED_BLOB_SIZE];
		// Minimal and maximal level of detail
		short lod_count = 0, max_lod = -1, min_lod = -1;
		// Determine whether all data types have been completely read
		bool found_header = false, finished[3] = { false, false, false };
		// File position to resume reading at
		fpos_t node_pos, way_pos, relation_pos;

		// input file to read from
		FILE *fp;
		errno_t err = fopen_s(&fp, m_input.data(), "rb");

		if (err == 0)
			logger.Log(LogLvl::info, "File " + m_input + " has been successfully opened");
		else
			throw io_error("Data input file could not be opened");

		while (!finished[node] || !finished[way] || !finished[relation])
		{
			int32_t size;
			bool found_data = false, skip_blob = false, eof = false;

			OSMPBF::BlobHeader blob_header = OSMPBF::BlobHeader();
			OSMPBF::Blob blob = OSMPBF::Blob();

			fpos_t before_blob;
			fgetpos(fp, &before_blob);

			// Check if EOF has been reached
			if (fread_s(&size, sizeof(int32_t), sizeof(int32_t), 1, fp) != 1)
				eof = true;
			else
				fsetpos(fp, &before_blob);

			// Write all data read to specified file when all vectors are full
			if ((!m_read_type[node] && !m_read_type[way]) || eof)
			{
				CleanOutData();

				// Set stream position to stored position depending on what has
				// already been completely read
				if (eof && m_read_type[relation])
				{
					finished[relation] = true;
					SetReadType(relation, false);
					logger.Log(LogLvl::info, "Finished reading Relations");
					eof = false;
				}

				// If nodes still need to be read
				if (!finished[node])
				{
					fsetpos(fp, &node_pos);
					logger.Log(LogLvl::debug, "Commencing reading nodes");
					SetReadType(node, true);
					SetReadType(way, true);
					SetReadType(relation, true);
				}
				// If ways still need to be read
				else if (!finished[way])
				{
					fsetpos(fp, &way_pos);
					logger.Log(LogLvl::debug, "Commencing reading ways");
					SetReadType(way, true);
					SetReadType(relation, true);
				}
				// If relations still need to be read
				else if (!finished[relation])
				{
					fsetpos(fp, &relation_pos);
					logger.Log(LogLvl::debug, "Commencing reading relations");
					SetReadType(relation, true);
				}

				char c;
				fpos_t tmp;
				fgetpos(fp, &tmp);

				// Check if EOF has been reached
				if (fread_s(&c, sizeof(char), sizeof(char), 1, fp) != 1)
					eof = true;
				else
					fsetpos(fp, &tmp);

				if (eof)
				{
					// If we reached the end of the file we finished reading objects
					if (!finished[node] && m_read_type[node])
					{
						finished[node] = true;
						logger.Log(LogLvl::info, "Finished reading Nodes");
					}
					if (!finished[way] && m_read_type[way] && finished[node])
					{
						finished[way] = true;
						logger.Log(LogLvl::info, "Finished reading Ways");
					}
					if (!finished[relation] && m_read_type[relation] && finished[way])
					{
						finished[relation] = true;
						logger.Log(LogLvl::info, "Finished reading Relations");
					}

					logger.Log(LogLvl::info, "Reached end of File");

					delete[] unpack_buffer;
					delete[] buffer;

					WriteMetaFile(lod_count);
					CleanUp();

					return;
				}
			}

			// Read first 4 bytes
			if (fread_s(&size, sizeof(int32_t), sizeof(int32_t), 1, fp) != 1)
				throw data_error("Incorrect Blob size");

			// Convert the size from network byte-order to host byte-order
			size = ntohl(size);
			if (size > MAX_BLOB_HEADER_SIZE)
				throw data_error("Blob Header is too big");

			if (fread(buffer, size, 1, fp) != 1)
				throw data_error("Unable to read Blob Header");

			if (!blob_header.ParseFromArray(buffer, size))
				throw data_error("Unable to read Blob Header");

			logger.Log(LogLvl::debug, "BlobHeader: " + std::to_string(size) + " bytes, type: " + blob_header.type());

			size = blob_header.datasize();
			logger.Log(LogLvl::debug, 1, "datasize: " + std::to_string(size) + " bytes");

			if (blob_header.has_indexdata())
				logger.Log(LogLvl::debug, 1, "indexdata = " + blob_header.indexdata().size());

			if (size > MAX_UNCOMPRESSED_BLOB_SIZE)
				throw data_error("Blob is too big");

			if (fread(buffer, size, 1, fp) != 1)
				throw data_error("Unable to read Blob data");

			if (!blob.ParseFromArray(buffer, size))
				throw data_error("Unable to read Blob data");

			logger.Log(LogLvl::debug, "Blob: " + std::to_string(size) + " bytes");

			if (blob.has_raw())
			{
				found_data = true;

				if (size != blob.raw().size())
					throw data_error("Blob size not as reported");

				size = blob.raw().size();

				logger.Log(LogLvl::debug, 1, "uncompressed data in blob: " + std::to_string(size) + " bytes");

				memcpy(unpack_buffer, buffer, size);
			}
			if (blob.has_zlib_data())
			{
				if (found_data)
					logger.Log(LogLvl::warning, "Found more than one data stream");
				else
					found_data = true;

				size = blob.zlib_data().size();
				logger.Log(LogLvl::debug, 1, "compressed data in blob: " + std::to_string(size) +
					" bytes, uncompressed size: " + std::to_string(blob.raw_size()) + " bytes");

				z_stream z;
				z.next_in = (unsigned char*)blob.zlib_data().c_str();
				z.avail_in = size;
				z.next_out = (unsigned char*)unpack_buffer;
				z.avail_out = blob.raw_size();

				z.zalloc = Z_NULL;
				z.zfree = Z_NULL;
				z.opaque = Z_NULL;

				if (inflateInit(&z) != Z_OK)
					skip_blob = true;

				if (inflate(&z, Z_FINISH) != Z_STREAM_END)
					skip_blob = true;

				if (inflateEnd(&z) != Z_OK)
					skip_blob = true;

				size = z.total_out;

				if (skip_blob)
					size = blob.zlib_data().size();
			}
			if (!found_data)
				logger.Log(LogLvl::error, "Missing data stream");

			if (skip_blob)
			{
				logger.Log(LogLvl::error, "Error occured while extracting data - Blob will be skipped");
				fseek(fp, size, SEEK_CUR);
			}
			else if (blob_header.type().compare("OSMHeader") == 0)
			{
				if (found_header)
				{
					logger.Log(LogLvl::error, "Found more than one header - skipping Blob");
					fseek(fp, size, SEEK_CUR);
					continue;
				}

				found_header = true;
				logger.Log(LogLvl::debug, "Blob Type: HeaderBlock (OSMHeader)");

				OSMPBF::HeaderBlock header_block = OSMPBF::HeaderBlock();

				if (!header_block.ParseFromArray(unpack_buffer, size))
				{
					logger.Log(logging::LogLvl::error, "Unable to parse header block");
					CleanUp();
					return;
				}

				if (header_block.has_bbox())
				{
					const OSMPBF::HeaderBBox bbox = header_block.bbox();

					m_minlon = (double)bbox.left() / LONLAT_RESOLUTION;
					m_minlat = (double)bbox.bottom() / LONLAT_RESOLUTION;
					m_maxlon = (double)bbox.right() / LONLAT_RESOLUTION;
					m_maxlat = (double)bbox.top() / LONLAT_RESOLUTION;
				}
				else
				{
					m_minlat = -90.0;
					m_maxlat = 90.0;
					m_minlon = -180.0;
					m_maxlon = 180.0;
				}

				for (int i = C_MAX_LOD; i >= C_MIN_LOD; i--)
				{
					if (m_lods[i] != 0)
					{
						lod_count++;
						min_lod = i;
						if (max_lod == -1)
							max_lod = i;
					}
				}
				logger.Log(LogLvl::debug, 1, "Set maximal LoD of: " + std::to_string(max_lod));
				logger.Log(LogLvl::debug, 1, "Set minimal LoD of: " + std::to_string(min_lod));

				for (int i = 0, l = header_block.required_features_size(); i < l; i++)
				{
					logger.Log(LogLvl::info, 1, "required feature: " + header_block.required_features(i));
				}

				for (int i = 0, l = header_block.optional_features_size(); i < l; i++)
				{
					logger.Log(LogLvl::info, 1, "optional feature: " + header_block.optional_features(i));
				}

				if (header_block.has_writingprogram())
					logger.Log(LogLvl::info, 1, "writingprogram: " + header_block.writingprogram());

				if (header_block.has_source())
					logger.Log(LogLvl::info, 1, "writingprogram: " + header_block.source());

			}
			else if (blob_header.type().compare("OSMData") == 0)
			{
				if (!found_header)
					throw data_error("Invalid file structure");

				logger.Log(LogLvl::debug, "Blob Type: PrimitiveBlock (OSMData)");

				OSMPBF::PrimitiveBlock prim_block = OSMPBF::PrimitiveBlock();

				if (!prim_block.ParseFromArray(unpack_buffer, size))
				{
					logger.Log(logging::LogLvl::error, "Unable to parse primitive block");
					break;
				}

				logger.Log(LogLvl::debug, 1, "granularity: " + std::to_string(prim_block.granularity()));
				logger.Log(LogLvl::debug, 1, "lat_offset: " + std::to_string(prim_block.lat_offset()));
				logger.Log(LogLvl::debug, 1, "lon_offset: " + std::to_string(prim_block.lon_offset()));
				logger.Log(LogLvl::debug, 1, "date_granularity: " + std::to_string(prim_block.date_granularity()));
				logger.Log(LogLvl::debug, 1, "stringtable: " + std::to_string(prim_block.stringtable().s_size()) + " items");
				logger.Log(LogLvl::debug, 1, "primitivegroups: " + std::to_string(prim_block.primitivegroup_size()) + " groups");

				for (int i = 0, l = prim_block.primitivegroup_size(); i < l; i++)
				{
					bool found_nodes = false, found_ways = false, found_rels = false;
					OSMPBF::PrimitiveGroup prim_group = prim_block.primitivegroup(i);
					const RepeatedPtrField<std::string> string_table = prim_block.stringtable().s();

					if (prim_group.nodes_size() > 0)
					{
						found_nodes = true;
						if (m_read_type[node])
						{
							// Restart and empty vector if necessary
							if (CheckRestart(node, prim_group.nodes_size(), size) ||
								!ReadNodes(string_table, prim_block, prim_group))
							{
								node_pos = before_blob;
								SetReadType(node, false);
							}
						}
					}
					if (prim_group.has_dense())
					{
						found_nodes = true;
						if (m_read_type[node])
						{
							// Restart and empty vector if necessary
							if (CheckRestart(node, prim_group.dense().lat_size(), size) ||
								!ReadDenseNodes(string_table, prim_block, prim_group))
							{
								node_pos = before_blob;
								SetReadType(node, false);
							}
						}
					}
					if (prim_group.ways_size() > 0)
					{
						found_ways = true;
						if (m_read_type[way])
						{
							// Restart and empty vector if necessary
							if (CheckRestart(way, prim_group.ways_size(), size))
								way_pos = before_blob;
							else
								ReadWays(string_table, prim_block, prim_group);
						}
					}
					if (prim_group.relations_size() > 0)
					{
						found_rels = true;
						if (m_read_type[relation])
						{
							// Restart and empty vector if necessary
							if (CheckRestart(relation, prim_group.relations_size(), size))
								relation_pos = before_blob;
							else
								ReadRelations(string_table, prim_block, prim_group);
						}
					}
					// Id we did not find any items in the primitive group log it
					if (!found_nodes && !found_ways && !found_rels)
						logger.Log(LogLvl::warning, "No items in primitive group");

					// If we expected nodes but didn't find any we finished reading them
					if (!finished[node] && !found_nodes && m_read_type[node])
					{
						finished[node] = true;
						SetReadType(node, false);
						logger.Log(LogLvl::info, "Finished reading Nodes");
					}
					// If we expected ways but didn't find any we finished reading them
					if (!finished[way] && m_read_type[way] && !found_ways && finished[node])
					{
						finished[way] = true;
						SetReadType(way, false);
						logger.Log(LogLvl::info, "Finished reading Ways");
					}
				}
			}
		}
	}

	// Reading different data types
	bool Converter::ReadNodes(const RepeatedPtrField<string> &string_table, OSMPBF::PrimitiveBlock &prim_block, OSMPBF::PrimitiveGroup &prim_group)
	{
		logger.Log(LogLvl::info, "Nodes: " + std::to_string(prim_group.nodes_size()));

		for (int i = 0; i < prim_group.nodes_size(); i++)
		{
			double lat = START * (double)(prim_block.lat_offset() + (prim_block.granularity() * prim_group.nodes(i).lat()));
			double lon = START * (double)(prim_block.lon_offset() + (prim_block.granularity() * prim_group.nodes(i).lon()));

			if (lat > 90.0 || lat < -90.0 || lon > 180.0 || lon < -180.0)
				logger.Log(LogLvl::error, 1, string("Invalid node coordinates in input file: (" + std::to_string(lat) + ", " + std::to_string(lon) + ")"));

			RepeatedField<uint32> node_keys = prim_group.nodes(i).keys();
			RepeatedField<uint32> node_values = prim_group.nodes(i).vals();

			for (int j = 0; j < prim_group.nodes(i).keys_size(); j++)
			{
				if (0 == string_table.Get(node_keys.Get(j)).compare("natural") && 0 == string_table.Get(node_values.Get(j)).compare("tree"))
				{
					if (m_singles.size() < MAX_NODES)
						m_singles.push_back(NodeX(m_nodes.size(), tree));
					else
						return false;

					break;
				}
				else if (0 == string_table.Get(node_keys.Get(j)).compare("highway") && 0 == string_table.Get(node_values.Get(j)).compare("street_lamp"))
				{
					if (m_singles.size() < MAX_NODES)
						m_singles.push_back(NodeX(m_nodes.size(), lamp));
					else
						return false;

					break;
				}
			}

			m_nodes.push_back(Node(lat, lon, prim_group.nodes(i).id()));
			m_node_map.insert(std::pair<long long, size_t>(prim_group.nodes(i).id(), (m_nodes.size() - 1)));
		}
		return true;
	}

	bool Converter::ReadDenseNodes(const RepeatedPtrField<string> &string_table, OSMPBF::PrimitiveBlock &prim_block, OSMPBF::PrimitiveGroup &prim_group)
	{
		logger.Log(LogLvl::info, "Dense Nodes: " + std::to_string(prim_group.dense().id_size()));

		RepeatedField<int64> dense_id = prim_group.dense().id();
		RepeatedField<int64> dense_lat = prim_group.dense().lat();
		RepeatedField<int64> dense_lon = prim_group.dense().lon();
		RepeatedField<int32> dense_keys = prim_group.dense().keys_vals();

		// Decode delta encoded dense Node ID's, latitude values and longitude values
		if ((prim_group.dense().id_size() == prim_group.dense().lat_size()) && (prim_group.dense().id_size() == prim_group.dense().lon_size()))
		{
			int pos = 0;
			google::protobuf::int64 last = 0, last_lon = 0, last_lat = 0, delta, delta_lon, delta_lat;

			for (int i = 0; i < prim_group.dense().id_size(); i++)
			{
				// Update delta
				delta = dense_id.Get(i);
				delta_lat = dense_lat.Get(i);
				delta_lon = dense_lon.Get(i);
				// Set new value for current node
				dense_id.Set(i, (delta + last));
				dense_lat.Set(i, (delta_lat + last_lat));
				dense_lon.Set(i, (delta_lon + last_lon));

				double lat = START * (double)(prim_block.lat_offset() + (prim_block.granularity() * dense_lat.Get(i)));
				double lon = START * (double)(prim_block.lon_offset() + (prim_block.granularity() * dense_lon.Get(i)));

				if (lat > 90.0 || lat < -90.0 || lon > 180.0 || lon < -180.0)
					logger.Log(LogLvl::error, 1, string("Invalid node coordinates in input file: (" + std::to_string(lat) + ", " + std::to_string(lon) + ")"));


				for (; pos < dense_keys.size() && dense_keys.Get(pos) == 0; pos++);

				// Check if the current Node is tagged as a tree or street lamp
				for (; pos < dense_keys.size() && dense_keys.Get(pos) != 0; pos+=2)
				{
					if (string_table.Get(dense_keys.Get(pos)).compare("natural") == 0 && string_table.Get(dense_keys.Get(pos + 1)).compare("tree") == 0)
					{
						if (m_singles.size() < MAX_NODES)
							m_singles.push_back(NodeX(m_nodes.size(), tree));
						else
							return false;
					}
					else if (string_table.Get(dense_keys.Get(pos)).compare("highway") == 0 && string_table.Get(dense_keys.Get(pos + 1)).compare("street_lamp") == 0)
					{
						if (m_singles.size() < MAX_NODES)
							m_singles.push_back(NodeX(m_nodes.size(), lamp));
						else
							return false;
					}
				}

				m_nodes.push_back(Node(lat, lon, delta + last));
				m_node_map.insert(std::pair<long long, size_t>(delta + last, m_nodes.size() - 1));

				//update last (= previous value)
				last = dense_id.Get(i);
				last_lat = dense_lat.Get(i);
				last_lon = dense_lon.Get(i);
			}
		}
		else
		{
			logger.Log(LogLvl::error, "Dense node array sizes not indentical");
		}

		return true;
	}

	bool Converter::ReadWays(const RepeatedPtrField<string> &string_table, OSMPBF::PrimitiveBlock &prim_block, OSMPBF::PrimitiveGroup &prim_group)
	{
		logger.Log(LogLvl::info, "Ways: " + std::to_string(prim_group.ways_size()));

		int corrupted = 0;
		// Get Node references (delta coded) as well as all tags (stored in stringtable) of all Ways
		for (int i = 0; i < prim_group.ways_size(); i++)
		{
			RepeatedField<int64> way_refs = RepeatedField<int64>(prim_group.ways(i).refs());
			RepeatedField<uint32> way_keys = prim_group.ways(i).keys();
			RepeatedField<uint32> way_values = prim_group.ways(i).vals();

			vector<long long> ids = vector<long long>();
			Type type = AssignType(string_table, way_keys, way_values);
			bool store = true;

			if (type != none && ((way_refs.size() > 1 && !types::IsAreaType(type)) || (types::IsAreaType(type) && way_refs.size() > 3)))
			{
				// delta decoding of references (x0, x1-x0, x2-x1)
				int64 last = 0, delta = 0;
				long long id = prim_group.ways(i).id();
				vector<size_t> tmp = vector<size_t>();

				for (int n = 0; n < way_refs.size(); n++)
				{
					delta = way_refs.Get(n);
					way_refs.Set(n, delta + last);
					ids.push_back(delta + last);

					// store the Node's position in our nodes vector
					if (store)
					{
						std::unordered_map<long long, size_t>::iterator node_it = m_node_map.find(delta + last);
						if (node_it != m_node_map.end())
							tmp.push_back(node_it->second);
						else
							store = false;
					}

					last = way_refs.Get(n);
				}

				// If the order is not clockwise make it so
				Way object = Way(tmp, id, type);

				if (store && !object.IsValid(way_refs.size()))
				{
					corrupted++;
					continue;
				}

				if (store)
				{
					// Store nodes in clockwise order
					if (object.IsArea())
						object.MakeClockwise(m_nodes);
					// Remove duplicates to reduce overhead and
					// prevent constructing faulty objects
					object.RemoveDuplicates(&m_nodes);
				}

				if (!store)
				{
					AddWayForLaterUse(object, ids);
				}
				else
				{
					m_ways.push_back(object);
					m_way_map.insert(std::pair<long long, size_t>(id, m_ways.size() - 1));
				}

				tmp.clear();
				ids.clear();
			}
			else
			{
				corrupted++;
			}
		}
		if (corrupted > 0)
			logger.Log(LogLvl::info, 1, "skipped ways: " + std::to_string(corrupted));

		return true;
	}

	bool Converter::ReadRelations(const RepeatedPtrField<string> &string_table, OSMPBF::PrimitiveBlock &prim_block, OSMPBF::PrimitiveGroup &prim_group)
	{
		size_t corrupted = 0;
		Type relation_type;
		bool later = false, store = true;
		vector<long long> ids = vector<long long>();

		logger.Log(LogLvl::info, "Relations: " + std::to_string(prim_group.relations_size()));

		for (int i = 0; i < prim_group.relations_size(); i++)
		{
			RepeatedField<uint32> rel_keys = prim_group.relations(i).keys();
			RepeatedField<uint32> rel_vals = prim_group.relations(i).vals();
			RepeatedField<int32> rel_roles = prim_group.relations(i).roles_sid();
			RepeatedField<int64> rel_memID = prim_group.relations(i).memids();
			RepeatedField<int32> rel_mem_type = prim_group.relations(i).types();

			ids = vector<long long>(rel_roles.size());

			relation_type = AssignRelationType(string_table, rel_keys, rel_vals);

			if (relation_type != none)
			{
				vector<size_t> refs = vector<size_t>();
				vector<MemberRole> roles = vector<MemberRole>();
				vector<Member> member = vector<Member>();
				long long id = prim_group.relations(i).id();

				for (int k = 0; k < rel_memID.size() && store; k++)
				{
					// delta decoding of references (x0, x1-x0, x2-x1)
					int64 last = 0, delta;
					delta = rel_memID.Get(k);
					rel_memID.Set(k, (delta + last));
					ids.push_back(delta + last);

					if (0 == string_table.Get(rel_roles.Get(k)).compare("inner"))
						roles.push_back(inner);
					else if (0 == string_table.Get(rel_roles.Get(k)).compare("outer"))
						roles.push_back(outer);
					else if (0 == string_table.Get(rel_roles.Get(k)).compare("main_stream") && relation_type == waterway)
						roles.push_back(main_stream);
					else if (0 == string_table.Get(rel_roles.Get(k)).compare("side_stream") && relation_type == waterway)
						roles.push_back(side_stream);
					else if (0 == string_table.Get(rel_roles.Get(k)).compare("street") && relation_type == street)
						roles.push_back(street_role);
					else { store = false; break; }

					if (rel_mem_type.Get(k) == OSMPBF::Relation_MemberType::Relation_MemberType_WAY)
					{
						std::unordered_map<long long, size_t>::iterator it = m_way_map.find(delta + last);
						if (it != m_way_map.end())
						{
							member.push_back(way);
							refs.push_back(it->second);
						}
						else
						{
							later = true;
						}
					}
					else if (rel_mem_type.Get(k) == OSMPBF::Relation_MemberType::Relation_MemberType_NODE)
					{
						std::unordered_map<long long, size_t>::iterator it = m_node_map.find(delta + last);
						if (it != m_node_map.end())
						{
							member.push_back(node);
							refs.push_back(it->second);
						}
						else
						{
							later = true;
						}
					}
					else if (rel_mem_type.Get(k) == OSMPBF::Relation_MemberType::Relation_MemberType_RELATION)
					{
						std::unordered_map<long long, size_t>::iterator it = m_rel_map.find(delta + last);
						if (it != m_rel_map.end())
						{
							member.push_back(relation);
							refs.push_back(it->second);
						}
						else
						{
							later = true;
						}
					}
					last = rel_memID.Get(k);
				}
				Relation object = Relation(refs, roles, member, relation_type, id);

				if ((store && !object.IsValid(rel_memID.size())) || (!later && !store) ||
					refs.empty() || roles.empty() || member.empty())
				{
					ids.clear();
					corrupted++;
					continue;
				}

				// Reverses the order of elements in all vectors of the relation
				object.MakeClockwise();

				if (later && store)
				{
					AddRelationForLaterUse(object, ids);
				}
				else
				{
					m_relations.push_back(object);
					m_rel_map.insert(std::pair<long long, size_t>(id, m_relations.size()));
				}

				ids.clear();
			}
		}
		if (corrupted > 0)
			logger.Log(LogLvl::info, 1, "skipped relations: " + std::to_string(corrupted));

		return true;
	}

	Type Converter::AssignType(const RepeatedPtrField<string> &string_table, RepeatedField<uint32> &keys, RepeatedField<uint32> &values)
	{
		unsigned short border = 0;
		bool is_area = 0;
		Type type = none;

		if (keys.empty())
			return types::empty;

		for (int i = 0; i < keys.size(); i++)
		{
			if (string_table.Get(keys.Get(i)).compare("area") == 0 && string_table.Get(values.Get(i)).compare("yes") == 0)
			{
				is_area = true;
			}
			else if (string_table.Get(keys.Get(i)).compare("building") == 0)
			{
				// set flag to HOUSE value
				if (string_table.Get(values.Get(i)).compare("detached") == 0 || string_table.Get(values.Get(i)).compare("hut") == 0 ||
					string_table.Get(values.Get(i)).compare("house") == 0 || string_table.Get(values.Get(i)).compare("bungalow") == 0 ||
					string_table.Get(values.Get(i)).compare("shed") == 0 || string_table.Get(values.Get(i)).compare("cabin") == 0 ||
					string_table.Get(values.Get(i)).compare("garage") == 0 || string_table.Get(values.Get(i)).compare("kiosk") == 0)
					type = detached;
				else
					type = apartments;
			}
			else if (string_table.Get(keys.Get(i)).compare("highway") == 0)
			{
				// comment
				if (string_table.Get(values.Get(i)).compare("primary") == 0 || string_table.Get(values.Get(i)).compare("secondary") == 0 ||
					string_table.Get(values.Get(i)).compare("motorway") == 0 || string_table.Get(values.Get(i)).compare("trunk") == 0)
					type = large_road;
				else if (string_table.Get(values.Get(i)).compare("path") == 0 || string_table.Get(values.Get(i)).compare("footway") == 0 ||
					string_table.Get(values.Get(i)).compare("pedestrian") == 0 || string_table.Get(values.Get(i)).compare("steps") == 0 ||
					string_table.Get(values.Get(i)).compare("track") == 0 || string_table.Get(values.Get(i)).compare("cycleway") == 0 ||
					string_table.Get(values.Get(i)).compare("bridleway") == 0)
					type = path;
				else if (string_table.Get(values.Get(i)).compare("living_street") == 0 || string_table.Get(values.Get(i)).compare("residential") == 0 ||
					string_table.Get(values.Get(i)).compare("unclassified") == 0 || string_table.Get(values.Get(i)).compare("service") == 0)
					type = small_road;
				else
					type = middle_road;
			}
			else if (string_table.Get(keys.Get(i)).compare("landuse") == 0)
			{
				if (string_table.Get(values.Get(i)).compare("vineyard") == 0 || string_table.Get(values.Get(i)).compare("village_green") == 0 ||
					string_table.Get(values.Get(i)).compare("grass") == 0 || string_table.Get(values.Get(i)).compare("meadow") == 0 ||
					string_table.Get(values.Get(i)).compare("greenfield") == 0 || string_table.Get(values.Get(i)).compare("recreation_ground") == 0 ||
					string_table.Get(values.Get(i)).compare("allotment") == 0)
					type = green_land;
				else if (string_table.Get(values.Get(i)).compare("forest") == 0 || string_table.Get(values.Get(i)).compare("orchard") == 0)
					type = forest;
				else if (string_table.Get(values.Get(i)).compare("farmland") == 0)
					type = farm_land;
				else if (string_table.Get(values.Get(i)).compare("basin") == 0 || string_table.Get(values.Get(i)).compare("reservior") == 0)
					type = water;
				else if (string_table.Get(values.Get(i)).compare("brownfield") == 0 || string_table.Get(values.Get(i)).compare("landfill") == 0 ||
					string_table.Get(values.Get(i)).compare("construction") == 0 || string_table.Get(values.Get(i)).compare("farmyard") == 0)
					type = bare_land;
				else if (string_table.Get(values.Get(i)).compare("cemetery") == 0)
					type = graveyard;
				else if (string_table.Get(values.Get(i)).compare("industrial") == 0 || string_table.Get(values.Get(i)).compare("port") == 0 ||
					string_table.Get(values.Get(i)).compare("railway") == 0)
					type = industry;
				else if (string_table.Get(values.Get(i)).compare("residential") == 0 || string_table.Get(values.Get(i)).compare("garages") == 0 ||
					string_table.Get(values.Get(i)).compare("retail") == 0 || string_table.Get(values.Get(i)).compare("commercial") == 0)
					type = residential;
			}
			else if (string_table.Get(keys.Get(i)).compare("natural") == 0)
			{
				// comment
				if (string_table.Get(values.Get(i)).compare("wood") == 0)
					type = forest;
				if (string_table.Get(values.Get(i)).compare("grassland") == 0 || string_table.Get(values.Get(i)).compare("scrub") == 0)
					type = green_land;
				else if (string_table.Get(values.Get(i)).compare("water") == 0 || string_table.Get(values.Get(i)).compare("wetland") == 0 ||
					string_table.Get(values.Get(i)).compare("bay") == 0 || string_table.Get(values.Get(i)).compare("glacier") == 0 ||
					string_table.Get(values.Get(i)).compare("hot_spring") == 0)
					type = water;
				else if (string_table.Get(values.Get(i)).compare("bare_rock") == 0 || string_table.Get(values.Get(i)).compare("scree") == 0 ||
					string_table.Get(values.Get(i)).compare("shingle") == 0 || string_table.Get(values.Get(i)).compare("sand") == 0 ||
					string_table.Get(values.Get(i)).compare("beach") == 0 || string_table.Get(values.Get(i)).compare("fell") == 0 ||
					string_table.Get(values.Get(i)).compare("heath") == 0 || string_table.Get(values.Get(i)).compare("moor") == 0)
					type = bare_land;
				else if (string_table.Get(values.Get(i)).compare("tree_row") == 0)
					type = tree_row;
				else if (string_table.Get(values.Get(i)) == "coastline")
					type = coast;
			}
			else if (string_table.Get(keys.Get(i)).compare("amenity") == 0)
			{
				if (string_table.Get(values.Get(i)).compare("grave_yard") == 0)
					type = graveyard;
				else if (string_table.Get(values.Get(i)).compare("university") == 0 || string_table.Get(values.Get(i)).compare("school") == 0 ||
					string_table.Get(values.Get(i)).compare("college") == 0 || string_table.Get(values.Get(i)).compare("kindergarten") == 0)
					type = residential;
			}
			else if (string_table.Get(keys.Get(i)).compare("waterway") == 0)
			{
				if (string_table.Get(values.Get(i)).compare("riverbank") == 0 || string_table.Get(values.Get(i)).compare("dock") == 0)
					type = water;
				else if (string_table.Get(values.Get(i)).compare("river") == 0 || string_table.Get(values.Get(i)).compare("stream") == 0 ||
					string_table.Get(values.Get(i)).compare("canal") == 0 || string_table.Get(values.Get(i)).compare("drain") == 0 ||
					string_table.Get(values.Get(i)).compare("ditch") == 0)
					type = waterway;
			}
			else if (string_table.Get(keys.Get(i)).compare("water") == 0)
			{
				type = water;
			}
			else if (string_table.Get(keys.Get(i)).compare("type") == 0)
			{
				if (string_table.Get(values.Get(i)).compare("waterway") == 0)
					type = waterway;
				else if (string_table.Get(values.Get(i)).compare("street") == 0)
					type = street;
			}
			else if (string_table.Get(keys.Get(i)).compare("leisure") == 0)
			{
				if (string_table.Get(values.Get(i)).compare("garden") == 0 || string_table.Get(values.Get(i)).compare("park") == 0 ||
					string_table.Get(values.Get(i)).compare("nature_reserve") == 0 || string_table.Get(values.Get(i)).compare("track") == 0 ||
					string_table.Get(values.Get(i)).compare("golf_course") == 0 || string_table.Get(values.Get(i)).compare("pitch") == 0 ||
					string_table.Get(values.Get(i)).compare("dog_park") == 0 || string_table.Get(values.Get(i)).compare("miniature_golf") == 0)
					type = green_land;
				else if (string_table.Get(values.Get(i)).compare("common") == 0)
					type = bare_land;
				else if (string_table.Get(values.Get(i)).compare("bird_hide") == 0 || string_table.Get(values.Get(i)).compare("bandstand") == 0)
					type = detached;
			}
			else if (string_table.Get(keys.Get(i)).compare("border_type") == 0)
			{
				if (string_table.Get(values.Get(i)).compare("township") == 0 || string_table.Get(values.Get(i)).compare("city") == 0 ||
					string_table.Get(values.Get(i)).compare("village") == 0)
					type = city;
				else if (string_table.Get(values.Get(i)).compare("state") == 0 || string_table.Get(values.Get(i)).compare("province") == 0)
					type = state;
			}
			else if (string_table.Get(keys.Get(i)).compare("boundary") == 0 && string_table.Get(values.Get(i)).compare("administrative") == 0)
			{
				type = boundary;
			}
			else if (string_table.Get(keys.Get(i)).compare("admin_level") == 0)
			{
				int bvalue = 0;
				try
				{
					bvalue = stoi(string_table.Get(values.Get(i)), nullptr, 10);
				}
				catch (invalid_argument)
				{
					cout << "Tag value in stringtable could not be convertred to an integer!" << endl;
				}
				catch (out_of_range)
				{
					cout << "Tag value in stringtable was out of integer range!" << endl;
				}

				switch (bvalue)
				{
					case 2: border = 1; break;
					case 4: border = 2; break;
					case 6: case 7: case 8: border = 3; break;
					default: type = none;
				}
			}
		}
		// If we found both boundary=administrative and admin_level=[(2|4|6|7|8|)]
		// then we know we found a border of interest.
		if (type == boundary)
		{
			switch (border)
			{
				case 1: return nation;
				case 2: return state;
				case 3: return city;
				default: return none;
			}
		}

		// If we found a street tagged as an area it's a plaza
		if (is_area && types::IsRoadType(type))
			type = plaza;

		return type == none ? types::empty : type;
	}

	types::Type Converter::AssignRelationType(const RepeatedPtrField<std::string> &string_table, RepeatedField<uint32> &keys, RepeatedField<uint32> &values)
	{
		Type t = AssignType(string_table, keys, values);

		switch (t)
		{
		case types::empty: case path: case small_road:
		case middle_road: case large_road:
			return none;
		}

		return t;
	}

	// Control flow
	bool Converter::CheckRestart(types::Member type, size_t datasize, size_t bytesize)
	{
		switch (type)
		{
		case node:
			if (m_nodes.size() + datasize > m_nodes.max_size() || m_nodes.size() + datasize > MAX_NODES)
				//sizeof(vector<Node>) + (m_nodes.size() * NODE_SIZE) + bytesize > 300000000)
			{
				SetReadType(node, false);
				logger.Log(LogLvl::debug, "Node vector needs to be emptied");
				return true;
			} break;
		case way:
			if (m_ways.size() + datasize > m_ways.max_size() || m_ways.size() + datasize > MAX_WAYS)
				//sizeof(vector<Way>) + (m_ways.size() * MEAN_WAYSIZE) + bytesize > 300000000)
			{
				SetReadType(way, false);
				logger.Log(LogLvl::debug, "Way vector needs to be emptied");
				return true;
			} break;
		case relation:
			if (m_relations.size() + datasize > m_relations.max_size() || m_ways.size() + datasize > MAX_RELATIONS)
				//sizeof(vector<Relation>) + (m_relations.size() * MEAN_RELSIZE) + bytesize > 300000000)
			{
				SetReadType(relation, false);
				logger.Log(LogLvl::debug, "Relation vector needs to be emptied");
				return true;
			} break;
		}
		return false;
	}

	void Converter::SetReadType(types::Member type, bool read)
	{
		m_read_type[type] = read;
	}

	// Storing incomplete objects
	void Converter::AddWayForLaterUse(Way &original, vector<long long> &ids)
	{
		if (m_ways_left.size() == m_ways_left.max_size() || m_ways_left_map.size() >= MAX_RELATIONS)
		{
			SetReadType(way, false);
		}
		else
		{
			WayX later = WayX(vector<Node>(original.refs.size()), original.id, original.type);
			for (size_t i = 0; i < original.refs.size(); i++)
			{
				// store the Node's position in our m_nodes vector
				std::unordered_map<long long, size_t>::iterator node_it = m_node_map.find(ids[i]);
				if (node_it != m_node_map.end())
					later.nodes[i] = m_nodes.at(node_it->second);
				else
					later.nodes[i] = Node(0.0, 0.0, ids[i]);
			}
			m_ways_left.push_back(later);
			m_ways_left_map.insert(std::pair<long long, size_t>(later.id, m_ways_left.size() - 1));
		}
	}

	types::WayX Converter::WayXFromWay(Way &from)
	{
		WayX add = WayX(vector<Node>(from.refs.size()), from.id, from.type);
		for (size_t j = 0; j < from.refs.size(); j++)
		{
			add.nodes[j] = m_nodes.at(from.refs[j]);
		}
		return add;
	}

	void Converter::AddRelationForLaterUse(Relation &original, vector<long long> &ids)
	{
		if (m_rels_left.size() == m_rels_left.max_size() || m_rels_left_map.size() >= (MAX_RELATIONS/ 2))
		{
			SetReadType(relation, false);
		}
		else
		{
			RelationX later = RelationX();
			later.id = original.id;
			later.type = original.type;

			for (size_t i = 0; i < original.member_types.size(); i++)
			{
				switch (original.member_types[i])
				{
					case node:
					{
						// store the Node's position in our nodes vector
						std::unordered_map<long long, size_t>::iterator node_it = m_node_map.find(ids.at(i));
						if (node_it != m_node_map.end())
							later.nodes.push_back(m_nodes.at(node_it->second));
						else
							later.nodes.push_back(Node(NULL, NULL, ids.at(i)));

						later.roles.push_back(Role(original.roles[i], node, later.nodes.size() - 1));
					} break;
					case way:
					{
						// store the Way's position in our ways vector
						std::unordered_map<long long, size_t>::iterator way_it = m_way_map.find(ids.at(i));
						if (way_it != m_way_map.end())
							m_ways_left.push_back(WayXFromWay(m_ways.at(way_it->second)));
						else
							m_ways_left.push_back(WayX(vector<Node>(), ids.at(i), none));

						later.ways.push_back(m_ways_left.back());
						later.roles.push_back(Role(original.roles[i], way, later.ways.size() - 1));

					} break;
					case relation:
					{
						// store the Relation's position in our relations vector
						std::unordered_map<long long, size_t>::iterator relation_it = m_rel_map.find(ids[i]);
						if (relation_it != m_rel_map.end())
						{
							RelationXFromRelation(relation_it->second);
						}
						else
						{
							m_rels_left.push_back(RelationX());
							m_rels_left.back().id = ids.at(i);
						}

						later.relations.push_back(m_rels_left.back());
						later.roles.push_back(Role(original.roles[i], relation, later.relations.size() - 1));
					} break;
				}
			}
		}
	}

	void Converter::RelationXFromRelation(size_t index)
	{
		RelationX add = RelationX();
		add.id = m_relations[index].id;
		add.type = m_relations[index].type;

		for (size_t j = 0; j < m_relations[index].refs.size(); j++)
		{
			switch (m_relations[index].member_types[j])
			{
				case node:
				{
					add.nodes.push_back(m_nodes.at(m_relations[index].refs[j]));
					add.roles.push_back(Role(m_relations[index].roles[j], node, add.nodes.size() - 1));
				} break;
				case way:
				{
					// TODO look at this again
					add.ways.push_back(WayXFromWay(m_ways.at(m_relations[index].refs[j])));
					add.roles.push_back(Role(m_relations[index].roles[j], way, add.ways.size() - 1));
				} break;
				case relation:
				{
					RelationXFromRelation(m_relations[index].refs[j]);
					add.relations.push_back(m_rels_left.back());
					add.roles.push_back(Role(m_relations[index].roles[j], relation, add.relations.size() - 1));
				}
			}
		}
		m_rels_left.push_back(add);
	}

	///////////////////////////////////////////////////////
	// Preparing and Writing Output
	///////////////////////////////////////////////////////
	bool Converter::InitTile(short lod, size_t &start, size_t &end)
	{
		logger.Log(LogLvl::info, "Initialising Tile " + std::to_string(lod));

		bool halfed = false;
		size_t square = m_lods[lod];

		if (end == 0 && start == 0)
		{
			end = square;
			while (end > (std::numeric_limits<size_t>::max() / end))
			{
				end /= 2;
				halfed = true;
			}

			m_tilecount = square * square;

			m_lon_step = (m_maxlon - m_minlon) / square;
			m_lat_step = (m_maxlat - m_minlat) / square;
		}

		m_tiles.clear();
		m_tiles.reserve((end * end) - start);

		logger.Log(LogLvl::debug, 1, "tile count: " + std::to_string((end * end) - start));

		for (size_t i = start; i < end; i++)
		{
			for (size_t j = start; j < end; j++)
			{
				m_tiles.push_back(Tile(m_minlat + i * m_lat_step, m_minlat + (i + 1) * m_lat_step, m_minlon + j * m_lon_step, m_minlon + (j + 1) * m_lon_step));
			}
		}

		start = end;
		end *= 2;

		// Return whether tile count was too great
		return halfed;
	}

	void Converter::CleanOutData()
	{
		bool toggle = false;
		for (short lod = C_MAX_LOD; lod >= C_MIN_LOD; lod--)
		{
			size_t start = 0, end = 0, tiles = 0;

			while (m_lods[lod] > 0)
			{
				bool stop = false;
				if (!InitTile(lod, start, end))
					stop = true;

				SortWays(lod, m_ways);
				SortRelations(lod, m_relations);

				/* TODO Implement this!
				if (!m_ways_left.empty())
				{
					SortLeftoverWays(lod);
					UpdateLeftoverWays();
					SortLeftoverWays(lod);
				}
				if (!m_rels_left.empty())
				{
					SortLeftoverRelations(lod);
					UpdateLeftoverRelations();
					SortLeftoverRelations(lod);
				}*/

				if (!m_update)
				{
					WriteDataToFile(lod);
					toggle = true;
				}
				else
				{
					UpdateDatabase(lod);
				}

				if (lod == C_MAX_LOD)
				{
					for (;tiles < m_tiles.size(); tiles++)
					{
						m_way_count += m_tiles[tiles].way_refs.size() + m_tiles[tiles].wayx_refs.size();
						m_relation_count += m_tiles[tiles].relation_refs.size() + m_tiles[tiles].relationx_refs.size();
					}
				}

				if (stop)
					break;
			}
		}
		if (toggle)
			m_update = true;
	}

	// Tile-Membership
	size_t Converter::FindTile(size_t object_index, types::Member mem)
	{
		double x_steps = 0.0, y_steps = 0.0;
		size_t sides = std::floor(sqrt(m_tilecount));

		switch (mem)
		{
			case node:
			{
				x_steps = std::floor((m_nodes[object_index].lon - m_minlon) / m_lon_step);
				y_steps = std::floor((m_nodes[object_index].lat - m_minlat) / m_lat_step);
			} break;
			case way:
			{
				if (m_sort == first_node)
				{
					x_steps = std::floor((m_nodes[m_ways[object_index].refs[0]].lon - m_minlon) / m_lon_step);
					y_steps = std::floor((m_nodes[m_ways[object_index].refs[0]].lat - m_minlat) / m_lat_step);
				}
				else
				{
					double lat = 0.0, lon = 0.0;
					GetLatLonForSearch(object_index, way, lat, lon);

					x_steps = std::floor((lon - m_minlon) / m_lon_step);
					y_steps = std::floor((lat - m_minlat) / m_lat_step);
				}
			} break;
			case relation:
			{
				double lat = 0.0, lon = 0.0;
				if (m_sort == first_node)
					m_relations[object_index].GetFirstLatLon(m_nodes, m_ways, m_relations, lat, lon);
				else
					GetLatLonForSearch(object_index, relation, lat, lon);

				x_steps = std::floor((lon - m_minlon) / m_lon_step);
				y_steps = std::floor((lat - m_minlat) / m_lat_step);
			} break;
		}

		size_t tile_index = (size_t)x_steps + ((size_t)y_steps * sides);

		if ((size_t)x_steps > std::numeric_limits<size_t>::max() - (size_t)(y_steps * sides) || tile_index > m_tiles.size())
			SetOverflow();
		else
			return tile_index == m_tiles.size() ? m_tiles.size() - 1 : tile_index;

		return 0;
	}

	void Converter::GetLatLonForSearch(size_t object_index, types::Member mem, double &lat, double &lon)
	{
		struct skip {
			size_t latskip;
			size_t lonskip;
			size_t index;

			skip()
			{
				latskip = lonskip = index = 0;
			}

			skip(size_t lon, size_t lat, size_t i)
			{
				latskip = lat;
				lonskip = lon;
				index = i;
			}
		};

		struct big {
			size_t size;
			vector<skip> *at;
		};

		size_t sides = std::floor(sqrt(m_tilecount));
		if (mem == way)
		{
			if (m_sort == most_nodes)
			{
				vector<skip> one = vector<skip>(), two = vector<skip>();
				vector<skip> three = vector<skip>(), four = vector<skip>();
				vector<skip> five = vector<skip>(), six = vector<skip>();
				vector<skip> seven = vector<skip>(), eight = vector<skip>(), nine = vector<skip>();

				double x_steps = std::floor((m_nodes[m_ways[object_index].refs[0]].lon - m_minlon) / m_lon_step);
				double y_steps = std::floor((m_nodes[m_ways[object_index].refs[0]].lat - m_minlat) / m_lat_step);
				one.push_back(skip(x_steps, y_steps * sides, m_ways[object_index].refs[0]));

				// Check all nodes and assign them to one of the four
				// possible tiles they can lie inside of
				for (size_t i = 0; i < m_ways[object_index].refs.size(); i++)
				{
					x_steps = std::floor((m_nodes[m_ways[object_index].refs[i]].lon - m_minlon) / m_lon_step);
					y_steps = std::floor((m_nodes[m_ways[object_index].refs[i]].lat - m_minlat) / m_lat_step);
					size_t skip_x = x_steps;
					size_t skip_y = y_steps * sides;

					if (skip_x == one.back().lonskip && skip_y == one.back().latskip)
						one.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (two.empty())
						two.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (skip_x == two.back().lonskip && skip_y == two.back().latskip)
						two.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (three.empty())
						three.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (skip_x == three.back().lonskip && skip_y == three.back().latskip)
						three.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (four.empty())
						four.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (skip_x == four.back().lonskip && skip_y == four.back().latskip)
						four.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (five.empty())
						five.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (skip_x == five.back().lonskip && skip_y == five.back().latskip)
						five.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (six.empty())
						six.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (skip_x == six.back().lonskip && skip_y == six.back().latskip)
						six.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (seven.empty())
						seven.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (skip_x == seven.back().lonskip && skip_y == seven.back().latskip)
						seven.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (eight.empty())
						eight.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (skip_x == eight.back().lonskip && skip_y == eight.back().latskip)
						eight.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (nine.empty())
						nine.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else if (skip_x == nine.back().lonskip && skip_y == nine.back().latskip)
						nine.push_back(skip(skip_x, skip_y, m_ways[object_index].refs[i]));
					else
						throw logic_error("Not enough tile possibilities in GetLatLonForSearch()");
				}

				std::array<big, 9> sizes = { big{ one.size(), &one },
											 big{ two.size(), &two },
											 big{ three.size(), &three },
											 big{ four.size(), &four },
										 	 big{ five.size(), &five },
										 	 big{ six.size(), &six },
											 big{ seven.size(), &seven },
											 big{ eight.size(), &eight },
											 big{ nine.size(), &nine }};

				auto max = std::max_element(sizes.begin(), sizes.end(), [](const big &a, const big &b) { return a.size < b.size; });

				lat = m_nodes[max->at->back().index].lat;
				lon = m_nodes[max->at->back().index].lon;
			}
			else if (m_sort == subdivide)
			{
				double x_steps = std::floor((m_nodes[m_ways[object_index].refs[0]].lon - m_minlon) / m_lon_step);
				double y_steps = std::floor((m_nodes[m_ways[object_index].refs[0]].lat - m_minlat) / m_lat_step);
				skip one = skip(x_steps, y_steps * sides, m_ways[object_index].refs[0]);

				for (size_t i = 1; i < m_ways[object_index].refs.size(); i++)
				{
					if (!m_nodes[m_ways[object_index].refs[i]].IsInsideTile(m_tiles[one.lonskip + one.latskip]))
					{
						if (m_ways[object_index].IsArea())
							SubdivideArea(m_tiles[one.lonskip + one.latskip], m_ways[object_index]);
						else
							SubdivideLine(m_tiles[one.lonskip + one.latskip], m_ways[object_index]);

						break;
					}
				}
				lat = m_nodes[m_ways[object_index].refs[0]].lat;
				lon = m_nodes[m_ways[object_index].refs[0]].lon;
			}
		}
		else
		{
			if (m_sort == most_nodes)
			{
				vector<skip> one = vector<skip>(), two = vector<skip>();
				vector<skip> three = vector<skip>(), four = vector<skip>();
				vector<skip> five = vector<skip>(), six = vector<skip>();
				vector<skip> seven = vector<skip>(), eight = vector<skip>(), nine = vector<skip>();

				double flat = 0.0, flon = 0.0;
				m_relations[object_index].GetFirstLatLon(m_nodes, m_ways, m_relations, flat, flon);

				double x_steps = std::floor((flon - m_minlon) / m_lon_step);
				double y_steps = std::floor((flat - m_minlat) / m_lat_step);

				one.push_back(skip(x_steps, y_steps * sides, 0));

				for (size_t i = 0; i < m_relations[object_index].refs.size(); i++)
				{
					m_relations[object_index].GetLatLonAt(m_nodes, m_ways, m_relations, flat, flon, i);

					x_steps = std::floor((flon - m_minlon) / m_lon_step);
					y_steps = std::floor((flat - m_minlat) / m_lat_step);

					size_t skip_x = x_steps;
					size_t skip_y = y_steps * sides;

					if (skip_x == one.back().lonskip && skip_y == one.back().latskip)
						one.push_back(skip(skip_x, skip_y, i));
					else if (two.empty())
						two.push_back(skip(skip_x, skip_y, i));
					else if (skip_x == two.back().lonskip && skip_y == two.back().latskip)
						two.push_back(skip(skip_x, skip_y, i));
					else if (three.empty())
						three.push_back(skip(skip_x, skip_y, i));
					else if (skip_x == three.back().lonskip && skip_y == three.back().latskip)
						three.push_back(skip(skip_x, skip_y, i));
					else if (four.empty())
						four.push_back(skip(skip_x, skip_y, i));
					else if (skip_x == four.back().lonskip && skip_y == four.back().latskip)
						four.push_back(skip(skip_x, skip_y, i));
					else if (five.empty())
						five.push_back(skip(skip_x, skip_y, i));
					else if (skip_x == five.back().lonskip && skip_y == five.back().latskip)
						five.push_back(skip(skip_x, skip_y,i));
					else if (six.empty())
						six.push_back(skip(skip_x, skip_y, i));
					else if (skip_x == six.back().lonskip && skip_y == six.back().latskip)
						six.push_back(skip(skip_x, skip_y, i));
					else if (seven.empty())
						seven.push_back(skip(skip_x, skip_y, i));
					else if (skip_x == seven.back().lonskip && skip_y == seven.back().latskip)
						seven.push_back(skip(skip_x, skip_y, i));
					else if (eight.empty())
						eight.push_back(skip(skip_x, skip_y, i));
					else if (skip_x == eight.back().lonskip && skip_y == eight.back().latskip)
						eight.push_back(skip(skip_x, skip_y, i));
					else if (nine.empty())
						nine.push_back(skip(skip_x, skip_y, i));
					else if (skip_x == nine.back().lonskip && skip_y == nine.back().latskip)
						nine.push_back(skip(skip_x, skip_y, i));
					else
						throw logic_error("Not enough tile possibilities in GetLatLonForSearch()");
				}
				std::array<big, 9> sizes = { big{ one.size(), &one },
										 	 big{ two.size(), &two },
											 big{ three.size(), &three },
											 big{ four.size(), &four },
											 big{ five.size(), &five },
											 big{ six.size(), &six },
											 big{ seven.size(), &seven },
											 big{ eight.size(), &eight },
											 big{ nine.size(), &nine }};

				auto max = std::max_element(sizes.begin(), sizes.end(), [](const big &a, const big &b) { return a.size < b.size; });

				m_relations[object_index].GetLatLonAt(m_nodes, m_ways, m_relations, lat, lon, max->at->back().index);
			}
			else if (m_sort == subdivide)
			{
				double flat = 0.0, flon = 0.0;
				m_relations[object_index].GetFirstLatLon(m_nodes, m_ways, m_relations, flat, flon);

				double x_steps = std::floor((flon - m_minlon) / m_lon_step);
				double y_steps = std::floor((flat - m_minlat) / m_lat_step);
				skip one = skip(x_steps, y_steps * sides, 0);

				for (size_t i = 1; i < m_relations[object_index].refs.size(); i++)
				{
					m_relations[object_index].GetLatLonAt(m_nodes, m_ways, m_relations, flat, flon, i);

					x_steps = std::floor((flon - m_minlon) / m_lon_step);
					y_steps = std::floor((flat - m_minlat) / m_lat_step);

					if (x_steps != one.lonskip || (y_steps * sides) != one.latskip)
					{
						if (one.lonskip <= (std::numeric_limits<size_t>::max() - one.latskip) && (one.lonskip + one.latskip) < m_tiles.size())
							SubdivideRelation(m_tiles[one.lonskip + one.latskip], m_relations[object_index]);

						m_relations[object_index].GetFirstLatLon(m_nodes, m_ways, m_relations, lat, lon);

						return;
					}
				}
				m_relations[object_index].GetFirstLatLon(m_nodes, m_ways, m_relations, lat, lon);
			}
		}
	}

	void Converter::SetOverflow()
	{
		m_overflow = true;
	}

	bool Converter::GetOverflow()
	{
		bool before = m_overflow;
		m_overflow = false;

		return before;
	}

	// Metafile
	void Converter::WriteMetaFile(short num_lods)
	{
		FILE* file;
		errno_t err = fopen_s(&file, (m_output + "\\meta").data(), "wb");
		if (err == 0)
			logger.Log(LogLvl::info, "Opened meta file: " + (m_output + "\\meta"));
		else
			throw io_error("Meta file could not be opened");

		fwrite(reinterpret_cast<char*>(&m_major), sizeof(int), 1, file);
		fwrite(reinterpret_cast<char*>(&m_minor), sizeof(int), 1, file);
		fwrite(reinterpret_cast<char*>(&m_patch), sizeof(int), 1, file);

		fwrite(reinterpret_cast<char*>(&m_minlat), sizeof(double), 1, file);
		fwrite(reinterpret_cast<char*>(&m_maxlat), sizeof(double), 1, file);
		fwrite(reinterpret_cast<char*>(&m_minlon), sizeof(double), 1, file);
		fwrite(reinterpret_cast<char*>(&m_maxlon), sizeof(double), 1, file);

		fwrite(reinterpret_cast<char*>(&m_way_count), sizeof(size_t), 1, file);
		fwrite(reinterpret_cast<char*>(&m_relation_count), sizeof(size_t), 1, file);
		fwrite(reinterpret_cast<char*>(&m_sort), sizeof(int), 1, file);
		fwrite(reinterpret_cast<char*>(&m_line), sizeof(bool), 1, file);
		fwrite(reinterpret_cast<char*>(&num_lods), sizeof(short), 1, file);

		for (short i = 0; i < 16; i++)
		{
			if (m_lods[i] != 0)
				fwrite(reinterpret_cast<char*>(&i), sizeof(short), 1, file);
		}

		fclose(file);

		if (m_debug)
		{
			err = fopen_s(&file, (m_output + "\\meta.txt").data(), "w");
			if (err == 0)
				logger.Log(LogLvl::info, "Opened meta text file: " + (m_output + "\\meta.txt"));
			else
				throw io_error("Meta text file could not be opened");

			string s = string();
			switch (m_sort)
			{
				case first_node: s.assign("First Node"); break;
				case most_nodes: s.assign("Most Nodes"); break;
				case subdivide: s.assign("Subdivide"); break;
			}

			string l = m_line ? "Douglas-Peucker" : "Visvalingam-Whyatt";

			fprintf_s(file, "Version: %d.%d.%d\n", m_major, m_minor, m_patch);
			fprintf_s(file, "Bounding Box: %f %f %f %f\n", m_minlat, m_maxlat, m_minlon, m_maxlon);
			fprintf_s(file, "Max Ways: %Iu\n", m_way_count);
			fprintf_s(file, "Max Relations: %Iu\n", m_relation_count);
			fprintf_s(file, "Sorting: %d (%s)\n", m_sort, s.data());
			fprintf_s(file, "Line Simplification Algorithm: %d (%s)\n", m_line, l.data());
			fprintf_s(file, "LoD Count: %d\n", num_lods);

			fprintf_s(file, "LoDs:\n");
			for (short i = 0; i < 16; i++)
			{
				if (m_lods[i] != 0)
					fprintf_s(file, "\tlod %d\n", i);
			}

			fclose(file);
		}
	}

	// Data-Output
	void Converter::WriteDataToFile(short lod)
	{
		FILE *out;
		string lod_out = GetDataFilename(lod);
		errno_t err = fopen_s(&out, lod_out.data(), "a+b");
		if (err == 0)
			logger.Log(LogLvl::info, "Opened data file: " + lod_out);
		else
			throw io_error("Data output file could not be opened");

		FILE *look;
		string look_out = GetLookupFilename(lod);
		err = fopen_s(&look, look_out.data(), "a+b");
		if (err == 0)
			logger.Log(LogLvl::info, "Opened lookup file: " + look_out);
		else
			throw io_error("Lookup output file could not be opened");

		if (ftell(out) > 0)
			UpdateDatabase(lod);

		// Used to determine lookup sizes for the lookup file
		long int tile_prev = 0;

		size_t number_tiles = m_tiles.size();

		fwrite(reinterpret_cast<char*>(&number_tiles), sizeof(size_t), 1, look);
		fwrite(reinterpret_cast<char*>(&m_lat_step), sizeof(double), 1, look);
		fwrite(reinterpret_cast<char*>(&m_lon_step), sizeof(double), 1, look);

		if (lod == C_MAX_LOD && m_singles.empty())
			logger.Log(LogLvl::error, "No Singles Data");

		for (size_t i = 0; i < m_tiles.size(); i++)
		{
			// Write Tile header
			size_t wsize = m_tiles[i].solo_refs.size() + m_tiles[i].way_refs.size() + m_tiles[i].wayx_refs.size();
			size_t rsize = m_tiles[i].relation_refs.size() + m_tiles[i].relationx_refs.size();
			fwrite(reinterpret_cast<char*>(&wsize), sizeof(size_t), 1, out);
			fwrite(reinterpret_cast<char*>(&rsize), sizeof(size_t), 1, out);
			fwrite(reinterpret_cast<char*>(&m_tiles[i].min_lat), sizeof(double), 1, out);
			fwrite(reinterpret_cast<char*>(&m_tiles[i].max_lat), sizeof(double), 1, out);
			fwrite(reinterpret_cast<char*>(&m_tiles[i].min_lon), sizeof(double), 1, out);
			fwrite(reinterpret_cast<char*>(&m_tiles[i].max_lon), sizeof(double), 1, out);

			for (size_t t = 0; t < m_tiles[i].solo_refs.size(); t++)
			{
				// Write single object data
				size_t single_size = 1;
				fwrite(reinterpret_cast<char*>(&single_size), sizeof(size_t), 1, out);
				fwrite(reinterpret_cast<char*>(&m_singles[m_tiles[i].solo_refs[t]].type), sizeof(int), 1, out);
				fwrite(reinterpret_cast<char*>(&m_nodes[m_singles[m_tiles[i].solo_refs[t]].index].lat), sizeof(double), 1, out);
				fwrite(reinterpret_cast<char*>(&m_nodes[m_singles[m_tiles[i].solo_refs[t]].index].lon), sizeof(double), 1, out);
			}

			for (size_t j = 0; j < m_tiles[i].way_refs.size(); j++)
			{
				// Write Way data
				WriteWay(out, m_tiles[i].way_refs[j], true);
			}

			for (size_t j = 0; j < m_tiles[i].wayx_refs.size(); j++)
			{
				// Write leftover Way data
				WriteWayX(out, m_tiles[i].wayx_refs[j], true);
			}

			// Write Relation data
			for (size_t j = 0; j < m_tiles[i].relation_refs.size(); j++)
			{
				// Write Relation data
				WriteRelation(out, m_tiles[i].relation_refs[j], true);
			}

			for (size_t j = 0; j < m_tiles[i].relationx_refs.size(); j++)
			{
				// Write Relation data
				WriteRelationX(out, m_tiles[i].relationx_refs[j], true);
			}

			// Write Tile data into the lookup file
			fwrite(reinterpret_cast<char*>(&m_tiles[i].min_lat), sizeof(double), 1, look);
			fwrite(reinterpret_cast<char*>(&m_tiles[i].max_lat), sizeof(double), 1, look);
			fwrite(reinterpret_cast<char*>(&m_tiles[i].min_lon), sizeof(double), 1, look);
			fwrite(reinterpret_cast<char*>(&m_tiles[i].max_lon), sizeof(double), 1, look);
			fwrite(reinterpret_cast<char*>(&tile_prev), sizeof(long int), 1, look);

			tile_prev = ftell(out);
		}

		fclose(out);
		fclose(look);

		if (m_debug)
		{
			lod_out = GetDataFilename(lod) + ".txt";
			err = fopen_s(&out, lod_out.data(), "a+");
			if (err == 0)
				logger.Log(LogLvl::info, "Opened data text file: " + look_out);
			else
				throw io_error("Data output file could not be opened");

			look_out = GetLookupFilename(lod) + ".txt";
			err = fopen_s(&look, look_out.data(), "a+");
			if (err == 0)
				logger.Log(LogLvl::info, "Opened lookup text file: " + look_out);
			else
				throw io_error("Lookup output file could not be opened");

			size_t number_tiles = m_tiles.size();
			fprintf_s(look, "%Iu %f %f\n", number_tiles, m_lat_step, m_lon_step);

			// stores the number of lines one needs to read to get to this Tile
			long int tile_prev = 0;

			for (size_t i = 0; i < m_tiles.size(); i++)
			{
				// Write TILE HEADER
				size_t elements = m_tiles[i].solo_refs.size() + m_tiles[i].way_refs.size();
				fprintf_s(out, "%Iu %Iu %f %f %f %f\n", elements,
					m_tiles[i].relation_refs.size() + m_tiles[i].relationx_refs.size(),
					m_tiles[i].min_lat,
					m_tiles[i].max_lat,
					m_tiles[i].min_lon,
					m_tiles[i].max_lon);

				for (size_t t = 0; t < m_tiles[i].solo_refs.size(); t++)
				{
					// WRITE m_singles DATA
					fprintf_s(out, "%d %d\n%f %f\n", 1, m_singles.at(m_tiles[i].solo_refs[t]).type,
						m_nodes[m_singles[m_tiles[i].solo_refs[t]].index].lat, m_nodes[m_singles[m_tiles[i].solo_refs[t]].index].lon);
				}

				for (size_t j = 0; j < m_tiles[i].way_refs.size(); j++)
				{
					// Write Way data
					WriteWay(out, m_tiles[i].way_refs[j], false);
				}

				for (size_t j = 0; j < m_tiles[i].wayx_refs.size(); j++)
				{
					// Write leftover Way data
					WriteWayX(out, m_tiles[i].wayx_refs[j], false);
				}

				for (size_t j = 0; j < m_tiles[i].relation_refs.size(); j++)
				{
					// Write Relation data
					WriteRelation(out, m_tiles[i].relation_refs[j], false);
				}

				for (size_t j = 0; j < m_tiles[i].relationx_refs.size(); j++)
				{
					// Write Relation data
					WriteRelationX(out, m_tiles[i].relationx_refs[j], false);
				}

				// Write Tile data into the lookup file
				fprintf_s(look, "%f %f %f %f %d\n",
					m_tiles[i].min_lat,
					m_tiles[i].max_lat,
					m_tiles[i].min_lon,
					m_tiles[i].max_lon,
					tile_prev);

				tile_prev = ftell(out);
			}

			fclose(look);
			fclose(out);
		}
	}

	void Converter::UpdateDatabase(short lod)
	{
		FILE *out;
		string lod_out = m_output + "\\tmp_data";
		errno_t err = fopen_s(&out, lod_out.data(), "wb");
		if (err == 0)
			logger.Log(LogLvl::info, "Opened temporary data file: " + lod_out);
		else
			throw io_error("Temporary data output file could not be opened");

		FILE *read_out;
		lod_out = GetDataFilename(lod);
		err = fopen_s(&read_out, lod_out.data(), "rb");
		if (err == 0)
			logger.Log(LogLvl::info, "Opened data file: " + lod_out);
		else
			throw io_error("Data output file could not be opened");

		FILE *look;
		string look_out = m_output + "\\tmp_lookup";
			err = fopen_s(&look, look_out.data(), "wb");
		if (err == 0)
			logger.Log(LogLvl::info, "Opened temporary lookup file: " + look_out);
		else
			throw io_error("Temporary lookup output file could not be opened");

		FILE *read_look;
		look_out = GetLookupFilename(lod);
		err = fopen_s(&read_look, look_out.data(), "rb");
		if (err == 0)
			logger.Log(LogLvl::info, "Opened lookup file: " + look_out);
		else
			throw io_error("Lookup output file could not be opened");

		size_t num_tiles = 0;
		double latstep, lonstep;
		// Copy lookup file header
		fread_s(&num_tiles, sizeof(size_t), sizeof(size_t), 1, read_look);
		fread_s(&latstep, sizeof(double), sizeof(double), 1, read_look);
		fread_s(&lonstep, sizeof(double), sizeof(double), 1, read_look);

		fwrite(reinterpret_cast<char*>(&num_tiles), sizeof(size_t), 1, look);
		fwrite(reinterpret_cast<char*>(&latstep), sizeof(double), 1, look);
		fwrite(reinterpret_cast<char*>(&lonstep), sizeof(double), 1, look);

		for (size_t i = 0; i < num_tiles; i++)
		{
			double minlat, maxlat, minlon, maxlon;
			size_t num_ways, num_relations;

			fread_s(&num_ways, sizeof(size_t), sizeof(size_t), 1, read_out);
			fread_s(&num_relations, sizeof(size_t), sizeof(size_t), 1, read_out);
			fread_s(&minlat, sizeof(double), sizeof(double), 1, read_out);
			fread_s(&maxlat, sizeof(double), sizeof(double), 1, read_out);
			fread_s(&minlon, sizeof(double), sizeof(double), 1, read_out);
			fread_s(&maxlon, sizeof(double), sizeof(double), 1, read_out);

			size_t wsize = i < m_tiles.size() ? num_ways + m_tiles[i].way_refs.size() + m_tiles[i].solo_refs.size() : num_ways;
			size_t rsize = i < m_tiles.size() ? num_relations + m_tiles[i].relation_refs.size() : num_relations;

			fwrite(reinterpret_cast<char*>(&wsize), sizeof(size_t), 1, out);
			fwrite(reinterpret_cast<char*>(&rsize), sizeof(size_t), 1, out);
			fwrite(reinterpret_cast<char*>(&minlat), sizeof(double), 1, out);
			fwrite(reinterpret_cast<char*>(&maxlat), sizeof(double), 1, out);
			fwrite(reinterpret_cast<char*>(&minlon), sizeof(double), 1, out);
			fwrite(reinterpret_cast<char*>(&maxlon), sizeof(double), 1, out);

			// Copy existing way data to temporary file
			for (size_t j = 0; j < wsize; j++)
			{
				Type obj_type = none;
				size_t obj_size;

				fread_s(&obj_size, sizeof(size_t), sizeof(size_t), 1, read_out);
				fread_s(&obj_type, sizeof(int), sizeof(int), 1, read_out);
				fwrite(reinterpret_cast<char*>(&obj_size), sizeof(size_t), 1, out);
				fwrite(reinterpret_cast<char*>(&obj_type), sizeof(int), 1, out);

				for (size_t n = 0; n < obj_size; n++)
				{
					double lat, lon;
					fread_s(&lat, sizeof(double), sizeof(double), 1, read_out);
					fread_s(&lon, sizeof(double), sizeof(double), 1, read_out);
					fwrite(reinterpret_cast<char*>(&lat), sizeof(double), 1, out);
					fwrite(reinterpret_cast<char*>(&lon), sizeof(double), 1, out);
				}
			}

			if (i < m_tiles.size())
			{
				// Write new single object data
				for (size_t j = 0; j < m_tiles[i].solo_refs.size(); j++)
				{
					size_t single_size = 1;
					fwrite(reinterpret_cast<char*>(&single_size), sizeof(size_t), 1, out);
					fwrite(reinterpret_cast<char*>(&m_singles[m_tiles[i].solo_refs[j]].type), sizeof(int), 1, out);
					fwrite(reinterpret_cast<char*>(&m_nodes[m_singles[m_tiles[i].solo_refs[j]].index].lat), sizeof(double), 1, out);
					fwrite(reinterpret_cast<char*>(&m_nodes[m_singles[m_tiles[i].solo_refs[j]].index].lon), sizeof(double), 1, out);
				}

				// Write new way data
				for (size_t j = 0; j < m_tiles[i].way_refs.size(); j++)
				{
					WriteWay(out, m_tiles[i].way_refs[j], true);
				}

				// Write new leftover way data
				for (size_t j = 0; j < m_tiles[i].wayx_refs.size(); j++)
				{
					WriteWayX(out, m_tiles[i].wayx_refs[j], true);
				}
			}

			// Copy existing relation data to temporary file
			for (size_t j = 0; j < rsize; j++)
			{
				TransferRelation(read_out, out);
			}

			if (i < m_tiles.size())
			{
				// Write new relation data
				for (size_t j = 0; j < m_tiles[i].relation_refs.size(); j++)
				{
					WriteRelation(out, m_tiles[i].relation_refs[j], true);
				}

				// Write new leftover relation data
				for (size_t j = 0; j < m_tiles[i].relationx_refs.size(); j++)
				{
					WriteRelationX(out, m_tiles[i].relationx_refs[j], true);
				}
			}

			long int skip = 0;

			fread_s(&minlat, sizeof(double), sizeof(double), 1, read_look);
			fread_s(&maxlat, sizeof(double), sizeof(double), 1, read_look);
			fread_s(&minlon, sizeof(double), sizeof(double), 1, read_look);
			fread_s(&maxlon, sizeof(double), sizeof(double), 1, read_look);
			fread_s(&skip, sizeof(long int), sizeof(long int), 1, read_look);

			skip = ftell(out);

			fwrite(reinterpret_cast<char*>(&minlat), sizeof(double), 1, look);
			fwrite(reinterpret_cast<char*>(&maxlat), sizeof(double), 1, look);
			fwrite(reinterpret_cast<char*>(&minlon), sizeof(double), 1, look);
			fwrite(reinterpret_cast<char*>(&maxlon), sizeof(double), 1, look);
			fwrite(reinterpret_cast<char*>(&skip), sizeof(long int), 1, look);
		}

		if (std::remove(GetDataFilename(lod).data()) == 0)
			std::rename((m_output + "\\tmp_data").data(), GetDataFilename(lod).data());
		else
			throw io_error("Unable to update database data file");

		if (std::remove(GetLookupFilename(lod).data()) == 0)
			std::rename((m_output + "\\tmp_lookup").data(), GetLookupFilename(lod).data());
		else
			throw io_error("Unable to update database lookup file");

		logger.Log(logging::LogLvl::info, string("Updated LoD %d of the database", lod));
	}

	void Converter::TransferRelation(FILE *from, FILE *to)
	{
		size_t elements = 0;
		Type rtype = none;

		fread_s(&elements, sizeof(size_t), sizeof(size_t), 1, from);
		fread_s(&rtype, sizeof(int), sizeof(int), 1, from);

		fwrite(reinterpret_cast<char*>(&elements), sizeof(size_t), 1, to);
		fwrite(reinterpret_cast<char*>(&rtype), sizeof(int), 1, to);

		for (size_t i = 0; i < elements; i++)
		{
			bool isway = true;

			fread_s(&isway, sizeof(bool), sizeof(bool), 1, from);
			fwrite(reinterpret_cast<char*>(&isway), sizeof(bool), 1, to);

			size_t mem_size = 0;
			MemberRole mem_role = outer;

			fread_s(&mem_role, sizeof(int), sizeof(int), 1, from);
			fread_s(&mem_size, sizeof(size_t), sizeof(size_t), 1, from);

			fwrite(reinterpret_cast<char*>(&mem_role), sizeof(int), 1, to);
			fwrite(reinterpret_cast<char*>(&mem_size), sizeof(size_t), 1, to);

			if (isway)
			{
				for (size_t j = 0; j < mem_size; j++)
				{
					double lat = 0.0, lon = 0.0;
					fread_s(&lat, sizeof(double), sizeof(double), 1, from);
					fread_s(&lon, sizeof(double), sizeof(double), 1, from);

					fwrite(reinterpret_cast<char*>(&lat), sizeof(double), 1, to);
					fwrite(reinterpret_cast<char*>(&lon), sizeof(double), 1, to);
				}
			}
			else
			{
				TransferMemberRelation(from, to, mem_size);
			}
		}
	}

	void Converter::TransferMemberRelation(FILE * from, FILE * to, size_t size)
	{
		for (size_t i = 0; i < size; i++)
		{
			bool isway = true;

			fread_s(&isway, sizeof(bool), sizeof(bool), 1, from);
			fwrite(reinterpret_cast<char*>(&isway), sizeof(bool), 1, to);

			size_t mem_size = 0;
			MemberRole mem_role = outer;

			fread_s(&mem_role, sizeof(int), sizeof(int), 1, from);
			fread_s(&mem_size, sizeof(size_t), sizeof(size_t), 1, from);

			fwrite(reinterpret_cast<char*>(&mem_role), sizeof(int), 1, to);
			fwrite(reinterpret_cast<char*>(&mem_size), sizeof(size_t), 1, to);

			if (isway)
			{
				for (size_t j = 0; j < mem_size; j++)
				{
					double lat = 0.0, lon = 0.0;
					fread_s(&lat, sizeof(double), sizeof(double), 1, from);
					fread_s(&lon, sizeof(double), sizeof(double), 1, from);

					fwrite(reinterpret_cast<char*>(&lat), sizeof(double), 1, to);
					fwrite(reinterpret_cast<char*>(&lon), sizeof(double), 1, to);
				}
			}
			else
			{
				TransferMemberRelation(from, to, mem_size);
			}
		}
	}

	void Converter::WriteNode(FILE *out, size_t index, bool b)
	{
		if (b)
		{
			fwrite(reinterpret_cast<char*>(&m_nodes.at(index).lat), sizeof(double), 1, out);
			fwrite(reinterpret_cast<char*>(&m_nodes.at(index).lon), sizeof(double), 1, out);
		}
		else
		{
			fprintf_s(out, "%.7f %.7f\n", m_nodes.at(index).lat, m_nodes.at(index).lon);
		}
	}

	void Converter::WriteWay(FILE *out, size_t index, bool b)
	{
		size_t node_count = m_ways[index].Size();

		if (b)
		{
			fwrite(reinterpret_cast<char*>(&node_count), sizeof(size_t), 1, out);
			fwrite(reinterpret_cast<char*>(&m_ways[index].type), sizeof(int), 1, out);
		}
		else
		{
			fprintf_s(out, "%Iu %d\n", node_count, m_ways[index].type);
		}

		for (size_t n = 0; n < node_count; n++)
		{
			WriteNode(out, m_ways[index].refs[n], b);
		}
	}

	void Converter::WriteWayX(FILE *out, size_t index, bool b)
	{
		size_t node_count = m_ways_left[index].nodes.size();
		if (b)
		{
			fwrite(reinterpret_cast<char*>(&node_count), sizeof(size_t), 1, out);
			fwrite(reinterpret_cast<char*>(&m_ways_left[index].type), sizeof(int), 1, out);
		}
		else
		{
			fprintf_s(out, "%Iu %d\n", node_count, m_ways_left[index].type);
		}

		for (size_t i = 0; i < m_ways_left[index].nodes.size(); i++)
		{
			if (b)
			{
				fwrite(reinterpret_cast<char*>(&m_ways_left[index].nodes[i].lat), sizeof(double), 1, out);
				fwrite(reinterpret_cast<char*>(&m_ways_left[index].nodes[i].lon), sizeof(double), 1, out);
			}
			else
			{
				fprintf_s(out, "%f %f\n", m_ways_left[index].nodes[i].lat, m_ways_left[index].nodes[i].lon);

			}
		}
	}

	void Converter::WriteRelation(FILE *out, size_t index, bool b)
	{
		size_t elements = m_relations[index].Size();

		if (b)
		{
			fwrite(reinterpret_cast<char*>(&elements), sizeof(size_t), 1, out);
			fwrite(reinterpret_cast<char*>(&m_relations[index].type), sizeof(int), 1, out);
		}
		else
		{
			fprintf_s(out, "%Iu %d\n", elements, m_relations[index].type);
		}

		for (size_t n = 0; n < elements; n++)
		{
			bool is_way_node = m_relations[index].member_types[n] != relation ? true : false;
			switch (m_relations[index].member_types[n])
			{
				case node:
				{
					if (b)
					{
						size_t one = 1;
						fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
						fwrite(reinterpret_cast<char*>(&m_relations[index].roles[n]), sizeof(int), 1, out);
						fwrite(reinterpret_cast<char*>(&one), sizeof(size_t), 1, out);
					}
					else
					{
						fprintf_s(out, "%d %d %d\n", m_relations[index].member_types[n], m_relations[index].roles[n], 1);
					}
					WriteNode(out, m_relations[index].refs[n], b);
				} break;
				case way:
				{
					if (b)
					{
						fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
						fwrite(reinterpret_cast<char*>(&m_relations[index].roles[n]), sizeof(int), 1, out);
					}
					else
					{
						fprintf_s(out, "%d %d ", m_relations[index].member_types[n], m_relations[index].roles[n]);
					}
					WriteMemberWay(out, m_relations[index].refs[n], b);
				} break;
				case relation:
				{
					if (b)
					{
						fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
						fwrite(reinterpret_cast<char*>(&m_relations[index].roles[n]), sizeof(int), 1, out);
					}
					else
					{
						fprintf_s(out, "%d %d ", m_relations[index].member_types[n], m_relations[index].roles[n]);
					}
					WriteMemberRelation(out, m_relations[index].refs[n], b);
				} break;
			}
		}
	}

	void Converter::WriteRelationX(FILE *out, size_t index, bool b)
	{
		size_t elements = m_rels_left[index].Size();

		if (b)
		{
			fwrite(reinterpret_cast<char*>(&elements), sizeof(size_t), 1, out);
			fwrite(reinterpret_cast<char*>(&m_rels_left[index].type), sizeof(int), 1, out);
		}
		else
		{
			fprintf_s(out, "%Iu %d\n", elements, m_rels_left[index].type);
		}

		for (size_t i = 0; i < m_rels_left[index].roles.size(); i++)
		{
			bool is_way_node = m_rels_left[index].roles[i].as != relation ? true : false;
			switch (m_rels_left[index].roles[i].vec)
			{
				case node:
				{
					if (b)
					{
						size_t one = 1;
						fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].roles[i].as), sizeof(int), 1, out);
						fwrite(reinterpret_cast<char*>(&one), sizeof(size_t), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].nodes[i].lat), sizeof(double), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].nodes[i].lon), sizeof(double), 1, out);
					}
					else
					{
						fprintf_s(out, "%d %d %Iu %f %f\n", is_way_node, m_rels_left[index].roles[i].as, 1,
										m_rels_left[index].nodes[i].lat, m_rels_left[index].nodes[i].lon);
					}
				} break;
				case way:
				{
					if (b)
					{
						fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].roles[i].as), sizeof(int), 1, out);
					}
					else
					{
						fprintf_s(out, "%d %d ", is_way_node, m_rels_left[index].roles[i].as);
					}
					WriteXMemberWay(out, index, i, b);
				} break;
				case relation:
				{
					if (b)
					{
						fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].roles[i].as), sizeof(int), 1, out);
					}
					else
					{
						fprintf_s(out, "%d %d ", is_way_node, m_rels_left[index].roles[i].as);
					}
					WriteXMemberRelation(out, index, i, b);
				} break;
			}
		}
	}

	void Converter::WriteXMemberWay(FILE *out, size_t index, size_t obj, bool b)
	{
		size_t elements = m_rels_left[index].ways[obj].Size();

		if (b)
			fwrite(reinterpret_cast<char*>(&elements), sizeof(size_t), 1, out);
		else
			fprintf_s(out, "%Iu\n", elements);

		for (size_t i = 0; i < elements; i++)
		{
			if (b)
			{
				fwrite(reinterpret_cast<char*>(&m_rels_left[index].ways[obj].nodes[i].lat), sizeof(double), 1, out);
				fwrite(reinterpret_cast<char*>(&m_rels_left[index].ways[obj].nodes[i].lon), sizeof(double), 1, out);
			}
			else
			{
				fprintf_s(out, "%f %f\n", m_rels_left[index].ways[obj].nodes[i].lat, m_rels_left[index].ways[obj].nodes[i].lon);
			}
		}
	}

	void Converter::WriteXMemberRelation(FILE *out, size_t index, size_t obj, bool b)
	{
		size_t elements = m_rels_left[index].relations[obj].Size();

		if (b)
			fwrite(reinterpret_cast<char*>(&elements), sizeof(size_t), 1, out);
		else
			fprintf_s(out, "%Iu\n", elements);

		for (size_t i = 0; i < elements; i++)
		{
			bool is_way_node = m_rels_left[index].relations[obj].roles[i].vec == relation ? false : true;
			switch (m_rels_left[index].relations[obj].roles[i].vec)
			{
				case node:
				{
					if (b)
					{
						size_t one = 1;
						fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].relations[obj].roles[i].as), sizeof(int), 1, out);
						fwrite(reinterpret_cast<char*>(&one), sizeof(size_t), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].relations[obj].nodes[i].lat), sizeof(double), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].relations[obj].nodes[i].lon), sizeof(double), 1, out);
					}
					else
					{
						fprintf_s(out, "%d %d %Iu %f %f\n", is_way_node, m_rels_left[index].relations[obj].roles[i].as, 1,
							m_rels_left[index].relations[obj].nodes[i].lat, m_rels_left[index].relations[obj].nodes[i].lon);
					}
				} break;
				case way:
				{
					if (b)
					{
						fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].relations[obj].roles[i].as), sizeof(int), 1, out);
					}
					else
					{
						fprintf_s(out, "%d %d ", is_way_node, m_rels_left[index].relations[obj].roles[i].as);
					}
					WriteXMemberWay(out, obj, i, b);
				} break;
				case relation:
				{
					if (b)
					{
						fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
						fwrite(reinterpret_cast<char*>(&m_rels_left[index].relations[obj].roles[i].as), sizeof(int), 1, out);
					}
					else
					{
						fprintf_s(out, "%d %d ", is_way_node, m_rels_left[index].relations[obj].roles[i].as);
					}
					WriteXMemberRelation(out, obj, i, b);
				} break;
			}
		}
	}

	void Converter::WriteMemberWay(FILE *out, size_t index, bool b)
	{
		size_t node_count = m_ways[index].Size();

		if (b)
			fwrite(reinterpret_cast<char*>(&node_count), sizeof(size_t), 1, out);
		else
			fprintf_s(out, "%Iu\n", node_count);

		for (size_t n = 0; n < node_count; n++)
		{
			WriteNode(out, m_ways[index].refs[n], b);
		}
	}

	void Converter::WriteMemberRelation(FILE *out, size_t index, bool b)
	{
		size_t elements = m_relations.at(index).Size();

		if (b)
			fwrite(reinterpret_cast<char*>(&elements), sizeof(size_t), 1, out);
		else
			fprintf_s(out, "%Iu\n", elements);

		for (size_t n = 0; n < m_relations[index].Size(); n++)
		{
			bool is_way_node = m_relations[index].member_types[n] != relation ? true : false;
			switch (m_relations[index].member_types[n])
			{
			case node:
			{
				if (b)
				{
					size_t one = 1;
					fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
					fwrite(reinterpret_cast<char*>(&m_relations[m_relations[index].roles[n]]), sizeof(int), 1, out);
					fwrite(reinterpret_cast<char*>(&one), sizeof(size_t), 1, out);
				}
				else
				{
					fprintf_s(out, "%d %d %d\n", m_relations[index].member_types[n], m_relations[index].roles[n], 1);
				}
				WriteNode(out, m_relations[index].refs[n], b);
			} break;
			case way:
			{
				if (b)
				{
					fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
					fwrite(reinterpret_cast<char*>(&m_relations[m_relations[index].roles[n]]), sizeof(int), 1, out);
				}
				else
				{
					fprintf_s(out, "%d %d", m_relations[index].member_types[n], m_relations[index].roles[n]);
				}
				WriteMemberWay(out, m_relations[index].refs[n], b);
			} break;
			case relation:
			{
				if (b)
				{
					fwrite(reinterpret_cast<char*>(&is_way_node), sizeof(bool), 1, out);
					fwrite(reinterpret_cast<char*>(&m_relations[m_relations[index].roles[n]]), sizeof(int), 1, out);
				}
				else
				{
					fprintf_s(out, "%d %d", m_relations[index].member_types[n], m_relations[index].roles[n]);
				}
				WriteMemberRelation(out, m_relations[index].refs[n], b);
			} break;
			}
		}
	}

	// Filenames
	string Converter::GetDataFilename(short lod)
	{
		string s = m_output;
		if (lod < 10)
			s += string("\\lod0") + std::to_string(lod);
		else
			s += string("\\lod") + std::to_string(lod);

		return s;
	}

	string Converter::GetLookupFilename(short lod)
	{
		string s = m_output;
		if (lod < 10)
			s += string("\\lookup0") + std::to_string(lod);
		else
			s += string("\\lookup") + std::to_string(lod);

		return s;
	}

	///////////////////////////////////////////////////////
	// Data Generalization
	///////////////////////////////////////////////////////
	size_t Converter::SubdivideLine(types::Tile &lod_tile, Way &object)
	{
		return Subdivide(lod_tile, object, false);
	}

	size_t Converter::SubdivideArea(types::Tile &lod_tile, Way &object)
	{
		return Subdivide(lod_tile, object, true);
	}

	size_t Converter::Subdivide(types::Tile &lod_tile, Way &object, bool save_last)
	{
		// Defines a line segment
		struct line {
			// First and last point that are inside or outside - depending on inside's value
			size_t first, last;
			bool inside;

			line()
			{
				line(0, 0, false);
			}
			line(size_t i, size_t j, bool b)
			{
				first = i;
				last = j;
				inside = b;
			}
		};

		if (save_last)
			object.refs.pop_back();

		vector<line> splits = vector<line>();
		int inside = 0, outside = 0;
		// Store indices of the points at which the polygon crosses the Tile border
		for (size_t i = 0; i < object.Size();)
		{
			splits.push_back(line());
			// If point is inside the tile
			if (m_nodes[object.refs[i]].IsInsideTile(lod_tile))
			{
				splits.back().first = i;
				inside++;

				i = i == object.Size() - 1 ? i : i + 1;

				int found = -1;
				for (; i < object.Size() && found < 0; i++)
				{
					if (!m_nodes[object.refs[i]].IsInsideTile(lod_tile) || object.Size() - 1)
						found = i - 1;
				}

				splits.back().last = found;
				splits.back().inside = true;
			}
			else
			{
				splits.back().first = i;
				outside++;

				double x_steps = std::floor((m_nodes[object.refs[i]].lon - m_minlon) / m_lon_step);
				double y_steps = std::floor((m_nodes[object.refs[i]].lat - m_minlat) / m_lat_step);

				size_t steps = x_steps + (sqrt(m_tilecount) * y_steps);

				if (!m_nodes[object.refs[i]].IsInsideTile(m_tiles[steps]))
					throw logic_error("Tile computation in partitioning was wrong");

				i = i == object.Size() - 1 ? i : i + 1;

				int found = -1;
				for (; i < object.Size() && found < 0; i++)
				{
					if (!m_nodes[object.refs[i]].IsInsideTile(m_tiles[steps]) || object.Size() - 1)
						found = i - 1;
				}

				splits.back().last = found;
				splits.back().inside = false;
			}
		}

		if (inside == 0 || outside == 0)
		{
			logger.Log(LogLvl::error, "Computed faulty line segments for subdivision");
			return 0;
		}

		bool saved_one = false;
		vector<size_t> inner_new = vector<size_t>();
		vector<Way> inner_lines = vector<Way>();
		vector<Way> outer_lines = vector<Way>();

		// Compute intersection points for all segments
		for (size_t i = 0; i < splits.size(); i++)
		{
			if (splits[i].inside)
			{
				// If the way we started with was an area we can store the new resulting area from all
				// inner line as one way
				if (save_last)
				{
					// Push all points that are inside the Tile into a vector
					for (size_t at = splits[i].first; at <= splits[i].last; at++)
					{
						inner_new.push_back(object.refs[at]);
					}
				}
				// If it was a line to begin with we can only store one of them and need to store the
				// others as new ways
				else
				{
					if (!saved_one)
					{
						if (!outer_lines.empty())
							inner_new.push_back(outer_lines.back().refs.back());

						// Push all points that are inside the Tile into a vector
						for (size_t at = splits[i].first; at <= splits[i].last; at++)
						{
							inner_new.push_back(object.refs[at]);
						}
					}
					else
					{
						if (!outer_lines.empty() && inner_lines.size() > 1)
							inner_lines.back().refs.push_back(outer_lines.back().refs.back());

						inner_lines.push_back(Way(vector<size_t>(), -2, object.type));

						// Push all points that are inside the Tile into a vector
						for (size_t at = splits[i].first; at <= splits[i].last; at++)
						{
							inner_lines.back().refs.push_back(object.refs[at]);
						}
					}
				}
			}
			else
			{
				bool found_before = false, found_after = false;
				types::Way subline = types::Way(vector<size_t>(), -2, object.type);

				Node intersect_one = Node(), intersect_two = Node();
				if (save_last || i > 0)
				{
					found_before = true;
					size_t other = splits[i].first == 0 ? (save_last ? object.Size() - 1 : 0) : splits[i].first - 1;
					intersect_one = mathtools::Intersection(lod_tile, m_nodes[object.refs[splits[i].first]], m_nodes[object.refs[other]]);
				}
				if (save_last || i < splits.size() - 1)
				{
					found_after = true;
					size_t other = splits[i].last == object.Size() - 1 ? (save_last ? 0 : object.Size() - 1) : splits[i].last + 1;
					intersect_two = mathtools::Intersection(lod_tile, m_nodes[object.refs[splits[i].last]], m_nodes[object.refs[other]]);
				}

				if ((found_before && intersect_one.id == -1) || (found_after && intersect_two.id == -1))
				{
					std::ofstream of("intersection.txt", ios_base::app);
					of.precision(12);

					of << "Object data:" << std::endl;
					for (size_t lol = 0; lol < object.Size(); lol++)
					{
						of << "\t lat: " << m_nodes[object.refs[lol]].lat << ", lon: " << m_nodes[object.refs[lol]].lon << std::endl;
					}
					of.close();
				}

				// Push first intersection point into new objects
				if (found_before)
				{
					m_nodes.push_back(intersect_one);
					subline.refs.push_back(m_nodes.size() - 1);
					if (save_last || (!saved_one && !inner_new.empty()))
						inner_new.push_back(m_nodes.size() - 1);
					else if (saved_one && !inner_lines.empty())
						inner_lines.back().refs.push_back(m_nodes.size() - 1);
				}

				// Push all nodes that lie in between, as well as and including, the first and
				// the last point into the subline's vector
				for (size_t at = splits[i].first; at <= splits[i].last; at++)
				{
					subline.refs.push_back(object.refs[at]);
				}

				// Push second intersection point into new way
				if (found_after)
				{
					m_nodes.push_back(intersect_two);
					subline.refs.push_back(m_nodes.size() - 1);
				}

				// If the original way is an area all new ways must also be areas
				if (save_last)
				{
					subline.refs.push_back(subline.refs[0]);
					inner_new.push_back(m_nodes.size() - 1);
				}
				else if (!saved_one && !inner_new.empty())
				{
					saved_one = true;
				}

				// Push new outer line way into way vector
				outer_lines.push_back(subline);
			}
		}
		if (save_last)
			inner_new.push_back(inner_new[0]);

		object.refs = inner_new;

		// Push all outer ways into the way vector
		for (size_t i = 0; i < outer_lines.size(); i++)
		{
			// TODO
			if (outer_lines[i].Size() > 0)
				m_ways.push_back(outer_lines[i]);
		}

		return outer_lines.size();
	}

	bool Converter::MergeAreas(short lod, types::Way &at, types::Way &other, vector<size_t> &store)
	{
		double threshold = GetLoDAreaSize(lod) / 2.0;

		// min lat, max lat, min lon, max lon
		Extrema one[4] = { Extrema() };
		Extrema two[4] = { Extrema() };

		for (size_t j = 0; j < at.refs.size(); j++)
		{
			if (m_nodes[at.refs[j]].lat <= one[0].lat)
				one[0].Update(m_nodes[at.refs[j]], j);
			if (m_nodes[at.refs[j]].lat >= one[1].lat)
				one[1].Update(m_nodes[at.refs[j]], j);
			if (m_nodes[at.refs[j]].lon <= one[2].lon)
				one[2].Update(m_nodes[at.refs[j]], j);
			if (m_nodes[at.refs[j]].lon >= one[3].lon)
				one[3].Update(m_nodes[at.refs[j]], j);
		}
		for (size_t j = 0; j < other.refs.size(); j++)
		{
			if (m_nodes[other.refs[j]].lat <= two[0].lat)
				two[0].Update(m_nodes[other.refs[j]], j);
			if (m_nodes[other.refs[j]].lat >= two[1].lat)
				two[1].Update(m_nodes[other.refs[j]], j);
			if (m_nodes[other.refs[j]].lon <= two[2].lon)
				two[2].Update(m_nodes[other.refs[j]], j);
			if (m_nodes[other.refs[j]].lon >= two[3].lon)
				two[3].Update(m_nodes[other.refs[j]], j);
		}

		double area = 0.0;
		switch (GetWayOrientation(one, two))
		{
			case Up:
				if ((area = vec2(one[1].lon, one[1].lat).Distance(vec2(two[0].lon, two[1].lat))) <= threshold)
				{
					store = Merge(at.refs, other.refs, one[3].index, one[2].index, two[2].index, two[3].index);
					return true;
				}
				else
				{
					//std::cout << "Area: " << area << ", Threshold: " << threshold << std::endl;
				} break;
			case Down:
				if ((area = vec2(one[0].lon, one[0].lat).Distance(vec2(two[1].lon, two[1].lat))) <= threshold)
				{
					store = Merge(at.refs, other.refs, one[2].index, one[3].index, two[3].index, two[2].index);
					return true;
				}
				else
				{
					//std::cout << "Area: " << area << ", Threshold: " << threshold << std::endl;
				} break;
			case Left:
				if ((area = vec2(one[2].lon, one[2].lat).Distance(vec2(two[3].lon, two[3].lat))) <= threshold)
				{
					store = Merge(at.refs, other.refs, one[1].index, one[0].index, two[0].index, two[1].index);
					return true;
				}
				else
				{
					//std::cout << "Area: " << area << ", Threshold: " << threshold << std::endl;
				} break;
			case Right:
				if ((area = vec2(one[3].lon, one[3].lat).Distance(vec2(two[2].lon, two[2].lat))) <= threshold)
				{
					store = Merge(at.refs, other.refs, one[0].index, one[1].index, two[1].index, two[0].index);
					return true;
				}
				else
				{
					//std::cout << "Area: " << area << ", Threshold: " << threshold << std::endl;
				} break;
			case UpperLeft:
				if ((area = vec2(one[1].lon, one[1].lat).Distance(vec2(two[3].lon, two[3].lat))) <= threshold ||
					(area = vec2(one[2].lon, one[2].lat).Distance(vec2(two[0].lon, two[0].lat))) <= threshold)
				{
					if (CloseCoordinates(one[1], two[3]))
						GetNewExtrema(Right, one[1], at, two[3], other);
					else if (CloseCoordinates(one[2], two[0]))
						GetNewExtrema(Left, one[2], at, two[0], other);

					store = Merge(at.refs, other.refs, one[1].index, one[2].index, two[0].index, two[3].index);
					return true;
				}
				else
				{
					//std::cout << "Area: " << area << ", Threshold: " << threshold << std::endl;
				} break;
			case UpperRight:
				if ((area = vec2(one[1].lon, one[1].lat).Distance(vec2(two[2].lon, two[2].lat))) <= threshold ||
					(area = vec2(one[3].lon, one[3].lat).Distance(vec2(two[0].lon, two[0].lat))) <= threshold)
				{
					if (CloseCoordinates(one[1], two[2]))
						GetNewExtrema(Left, one[1], at, two[2], other);
					else if (CloseCoordinates(one[3], two[0]))
						GetNewExtrema(Right, one[3], at, two[0], other);

					store = Merge(at.refs, other.refs, one[3].index, one[1].index, two[2].index, two[0].index);
					return true;
				}
				else
				{
					//std::cout << "Area: " << area << ", Threshold: " << threshold << std::endl;
				} break;
			case LowerLeft:
				if ((area = vec2(one[2].lon, one[2].lat).Distance(vec2(two[1].lon, two[1].lat))) <= threshold ||
					(area = vec2(one[0].lon, one[0].lat).Distance(vec2(two[3].lon, two[3].lat))) <= threshold)
				{
					if (CloseCoordinates(one[2], two[1]))
						GetNewExtrema(Left, one[2], at, two[1], other);
					else if (CloseCoordinates(one[0], two[3]))
						GetNewExtrema(Right, one[0], at, two[3], other);

					store = Merge(at.refs, other.refs, one[2].index, one[0].index, two[3].index, two[1].index);
					return true;
				}
				else
				{
					//std::cout << "Area: " << area << ", Threshold: " << threshold << std::endl;
				} break;
			case LowerRight:
				if ((area = vec2(one[3].lon, one[3].lat).Distance(vec2(two[1].lon, two[1].lat))) <= threshold ||
					(area = vec2(one[0].lon, one[0].lat).Distance(vec2(two[2].lon, two[2].lat))) <= threshold)
				{
					if (CloseCoordinates(one[3], two[1]))
						GetNewExtrema(Right, one[3], at, two[1], other);
					else if (CloseCoordinates(one[0], two[2]))
						GetNewExtrema(Left, one[0], at, two[2], other);

					store = Merge(at.refs, other.refs, one[0].index, one[3].index, two[1].index, two[2].index);
					return true;
				}
				else
				{
					//std::cout << "Area: " << area << ", Threshold: " << threshold << std::endl;
				} break;
		}
		return false;
	}

	Converter::WayOrientation Converter::GetWayOrientation(Converter::Extrema at[4], Converter::Extrema other[4])
	{
		// Upper Left
		if (other[0].lat >= at[1].lat && other[3].lon <= at[2].lon)
			return UpperLeft;
		else if (other[0].lat >= at[1].lat && other[2].lon >= at[3].lon)
			return UpperRight;
		else if (other[0].lat >= at[1].lat)
			return Up;
		else if (other[1].lat <= at[0].lat)
			return Down;
		else if (other[1].lat <= at[0].lat && other[3].lon <= at[2].lon)
			return LowerLeft;
		else if (other[1].lat <= at[0].lat && other[2].lon >= at[3].lon)
			return LowerRight;
		else if (other[3].lon <= at[2].lon)
			return Left;
		else if (other[2].lon >= at[3].lon)
			return Right;

		// Shut up the compiler
		return Up;
	}

	bool Converter::CloseCoordinates(Converter::Extrema &one, Converter::Extrema &two)
	{
		double dev = std::numeric_limits<double>::epsilon();
		return (one.lat == two.lat || one.lat == two.lat + dev || one.lat == two.lat - dev) &&
			   (one.lon == two.lon || one.lat == two.lon + dev || one.lon == two.lon - dev);

	}

	void Converter::GetNewExtrema(Converter::WayOrientation o, Converter::Extrema &one, types::Way &wone, Converter::Extrema &two, types::Way &wtwo)
	{
		switch (o)
		{
			case Left:
			{
				size_t prev = one.index > 0 ? one.index - 1 : wone.refs.size() - 1;
				size_t next = one.index < wone.refs.size() - 1 ? one.index + 1 : 0;

				if (m_nodes[wone.refs[prev]].lon < m_nodes[wone.refs[next]].lon)
				{
					one = Extrema(m_nodes[wtwo.refs[prev]], prev);
					next = two.index < wtwo.refs.size() - 1 ? two.index + 1 : 0;
					two = Extrema(m_nodes[wtwo.refs[next]], next);
				}
				else
				{
					one = Extrema(m_nodes[wtwo.refs[next]], next);
					prev = two.index > 0 ? two.index - 1 : wtwo.refs.size() - 1;
					two = Extrema(m_nodes[wtwo.refs[prev]], prev);
				}
			} break;
			case Right:
			{
				size_t prev = one.index > 0 ? one.index - 1 : wone.refs.size() - 1;
				size_t next = one.index < wone.refs.size() - 1 ? one.index - 1 : 0;

				if (m_nodes[wone.refs[prev]].lon > m_nodes[wone.refs[next]].lon)
				{
					one = Extrema(m_nodes[wtwo.refs[prev]], prev);
					next = two.index < wtwo.refs.size() - 1 ? two.index + 1 : 0;
					two = Extrema(m_nodes[wtwo.refs[next]], next);
				}
				else
				{
					one = Extrema(m_nodes[wtwo.refs[next]], next);
					prev = two.index > 0 ? two.index - 1 : wtwo.refs.size() - 1;
					two = Extrema(m_nodes[wtwo.refs[prev]], prev);
				}
			} break;
			case Up:
			{
				size_t prev = one.index > 0 ? one.index - 1 : wone.refs.size() - 1;
				size_t next = one.index < wone.refs.size() - 1 ? one.index - 1 : 0;

				if (m_nodes[wone.refs[prev]].lat > m_nodes[wone.refs[next]].lat)
				{
					one = Extrema(m_nodes[wtwo.refs[prev]], prev);
					next = two.index < wtwo.refs.size() - 1 ? two.index + 1 : 0;
					two = Extrema(m_nodes[wtwo.refs[next]], next);
				}
				else
				{
					one = Extrema(m_nodes[wtwo.refs[next]], next);
					prev = two.index > 0 ? two.index - 1 : wtwo.refs.size() - 1;
					two = Extrema(m_nodes[wtwo.refs[prev]], prev);
				}
			} break;
			case Down:
			{
				size_t prev = one.index > 0 ? one.index - 1 : wone.refs.size() - 1;
				size_t next = one.index < wone.refs.size() - 1 ? one.index - 1 : 0;

				if (m_nodes[wone.refs[prev]].lat < m_nodes[wone.refs[next]].lat)
				{
					one = Extrema(m_nodes[wtwo.refs[prev]], prev);
					next = two.index < wtwo.refs.size() - 1 ? two.index + 1 : 0;
					two = Extrema(m_nodes[wtwo.refs[next]], next);
				}
				else
				{
					one = Extrema(m_nodes[wtwo.refs[next]], next);
					prev = two.index > 0 ? two.index - 1 : wtwo.refs.size() - 1;
					two = Extrema(m_nodes[wtwo.refs[prev]], prev);
				}
			} break;
		}
	}

	std::vector<size_t> Converter::Merge(std::vector<size_t> &first, std::vector<size_t> &second, size_t a, size_t b, size_t c, size_t d)
	{
		vector<size_t> result = vector<size_t>();

		size_t index = a;
		while (index != b)
		{
			result.push_back(first[index]);
			index = index + 1 < first.size() ? index + 1 : 0;
		}
		result.push_back(first[b]);

		index = c;
		while (index != d)
		{
			result.push_back(second[index]);
			index = index + 1 < second.size() ? index + 1 : 0;
		}
		result.push_back(second[d]);
		result.push_back(result[0]);

		// Return vector containing merged polygon indices
		return result;
	}

	void Converter::ConstructConvexHull(std::vector<size_t> &points, size_t index)
	{
		vector<size_t> hull = vector<size_t>();

		size_t left = 0;
		for (size_t i = 1; i < points.size(); i++)
		{
			if (m_nodes[points[i]].lon < m_nodes[points[left]].lon)
				left = i;
		}

		size_t p = left, q;
		do
		{
			hull.push_back(points[p]);

			q = (p + 1) % points.size();
			for (size_t i = 0; i < points.size(); i++)
			{
				if (IsLeft(points[p], points[i], points[q]) < 0)
					q = i;
			}

			p = q;

		} while (p != left);

		m_ways[index].refs = hull;
	}

	double Converter::IsLeft(size_t start, size_t end, size_t at)
	{
		vec2 s = vec2(m_nodes[start]);
		vec2 e = vec2(m_nodes[end]);
		vec2 a = vec2(m_nodes[at]);

		return vec2::Orientation(s, e, a);
	}

	std::vector<size_t> Converter::DouglasPeucker(std::vector<size_t> line, double epsilon)
	{
		vector<size_t> result = vector<size_t>();
		vector<size_t> tmp1 = vector<size_t>();
		vector<size_t> tmp2 = vector<size_t>();

		size_t index = 0;
		double max = 0.0;

		if (line.empty())
			throw length_error("Tried to simplify empty line");

		// Find point of furthest Distance from the line that is drawn between the first
		// and the last point of the polygon/line
		size_t end = line.size() - 1;
		for (size_t i = 1; i < end; i++)
		{
			double dist = PerpendicularDistance(line, i);
			if (dist > max)
			{
				index = i;
				max = dist;
			}
		}

		// If the maximum Distance is greater than epsilon (threshold)
		// Account for precision loss using epsilon range around epsilon (lul)
		if (max > (epsilon - DOUBLE_EPSILON) || max > (epsilon + DOUBLE_EPSILON))
		{
			for (size_t j = 0; j <= index; j++)
			{
				tmp1.push_back(line[j]);
			}
			tmp1 = DouglasPeucker(tmp1, epsilon);

			for (size_t j = index; j <= end; j++)
			{
				tmp2.push_back(line[j]);
			}
			tmp2 = DouglasPeucker(tmp2, epsilon);

			// Build resulting vector
			for (size_t pos = 0; pos < tmp1.size() - 1; pos++)
			{
				result.push_back(tmp1[pos]);
			}
			for (size_t pos = 0; pos < tmp2.size(); pos++)
			{
				result.push_back(tmp2[pos]);
			}
		}
		else
		{
			result.push_back(line[0]);
			result.push_back(line[end]);
		}

		return result;
	}

	std::vector<size_t> Converter::VisvalingamWhyatt(std::vector<size_t> &line, size_t keep)
	{
		if (line.empty())
			throw length_error("Tried to simplify empty line");

		vector<size_t> tmp = vector<size_t>(line.begin(), line.end());

		while (tmp.size() > keep)
		{
			priority_queue<triangle, vector<triangle>, std::greater<triangle>> tris;

			for (size_t i = 1; i < tmp.size() - 2; i += 2)
			{
				triangle one = triangle(i, triangle::Area(vec2(m_nodes[tmp[i - 1]]), vec2(m_nodes[tmp[i]]), vec2(m_nodes[tmp[i + 1]])));

				if (i + 2 < tmp.size())
				{
					triangle two = triangle(i + 1, triangle::Area(vec2(m_nodes[tmp[i]]), vec2(m_nodes[tmp[i + 1]]), vec2(m_nodes[tmp[i + 2]])));

					vec2 intersect = vec2();
					if (mathtools::LineLineIntersection(
						vec2(m_nodes[tmp[i - 1]]),
						vec2(m_nodes[tmp[i + 1]]),
						vec2(m_nodes[tmp[i]]),
						vec2(m_nodes[tmp[i + 2]]), intersect))
					{
						double effective = triangle::Area(vec2(m_nodes[tmp[i]]), intersect, vec2(m_nodes[tmp[i + 1]]));
						one.area -= effective;
						two.area -= effective;
					}
					tris.push(two);
				}
				tris.push(one);
			}
			tmp.erase(tmp.begin() + tris.top().index);
		}

		return tmp;
	}

	double Converter::PerpendicularDistance(std::vector<size_t> &polygon, size_t current)
	{
		vec2 start = vec2(m_nodes.at(polygon[0]).lon, m_nodes.at(polygon[0]).lat);
		vec2 end = vec2(m_nodes.at(polygon.back()).lon, m_nodes.at(polygon.back()).lat);
		vec2 point = vec2(m_nodes.at(polygon[current]).lon, m_nodes.at(polygon[current]).lat);

		return point.PerpendicularDistance(start, end);
	}

	bool Converter::IsLoDType(short lod, types::Type t)
	{
		bool base = t != Type::empty && t != Type::none;

		switch (lod)
		{
		case 0: case 1: case 2:
			return base && ((types::IsAreaType(t) && !IsHouseType(t) && t != plaza) || t == nation); break;
		case 3: case 4: case 5: case 6: case 7:
			return base && t != plaza && !IsHouseType(t) && !IsRoadType(t) && t != tree_row && t != city; break;
		case 8:	case 9: case 10:
			return base && ((!IsHouseType(t) && t != tree_row && t != plaza) || t == large_road); break;
		case 11: case 12:
			return base && t != path && t != small_road && !IsHouseType(t) && t != tree_row; break;
		case 13: case 14:
			return base && t != path; break;
		case 15: return base; break;
		default: throw logic_error("Invail LoD number in lod type condition");
		}

		return base;
	}

	double Converter::GetLoDEpsilon(short lod)
	{
		switch (lod)
		{
			case  0: return 0.0016; break;
			case  1: return 0.0008; break;
			case  2: return 0.0004; break;
			case  3: return 0.0002; break;
			case  4: return 0.0001; break;
			case  5: return 0.00005; break;
			case  6: return 0.000025; break;
			case  7: return 0.0000125; break;
			case  8: return 0.00000625; break;
			case  9: return 0.000003125; break;
			case 10: return 0.0000015625; break;
			case 11: return 0.0000008; break;
			case 12: return 0.0000004; break;
			case 13: return 0.0000002; break;
			case 14: return 0.0000001; break;
			case 15: return 0.00000005; break;
			default: throw logic_error("Invail LoD number in epsilon computation");
		}
		// Shuts up the compiler
		return 0.0;
	}

	double Converter::GetLoDAreaSize(short lod)
	{
		// TODO: look up how to do this or test if this works
		switch (lod)
		{
			case  0: return 0.16; break;
			case  1: return 0.8; break;
			case  2: return 0.4; break;
			case  3: return 0.2; break;
			case  4: return 0.1; break;
			case  5: return 0.05; break;
			case  6: return 0.025; break;
			case  7: return 0.0125; break;
			case  8: return 0.00625; break;
			case  9: return 0.003125; break;
			case 10: return 0.0015625; break;
			case 11: return 0.0008; break;
			case 12: return 0.4; break;
			case 13: return 0.2; break;
			case 14: return 0.1; break;
			case 15: return 0.005; break;
			default: throw logic_error("Invail LoD number in max area size computation");
		}
		// Shuts up the compiler
		return 0.0;
	}

	size_t Converter::GetLoDPercentage(short lod, size_t size)
	{
		double percentage = 1.0;
		// TODO: look up how to do this or test if this works
		switch (lod)
		{
			case  0: percentage = 0.1; break;
			case  1: percentage = 0.15; break;
			case  2: percentage = 0.2; break;
			case  3: percentage = 0.25; break;
			case  4: percentage = 0.3; break;
			case  5: percentage = 0.35; break;
			case  6: percentage = 0.45; break;
			case  7: percentage = 0.55; break;
			case  8: percentage = 0.65; break;
			case  9: percentage = 0.7; break;
			case 10: percentage = 0.75; break;
			case 11: percentage = 0.8; break;
			case 12: percentage = 0.85; break;
			case 13: percentage = 0.9; break;
			case 14: percentage = 0.95; break;
			case 15: percentage = 1.0; break;
			default: throw logic_error("Invail LoD number in point percentage computation");
		}

		return size < 5 ? size : (size_t)std::floor((double)size * percentage);
	}

	// Way Generalization
	void Converter::GeneralizeWays(std::vector<Way> &objects, size_t index, short lod)
	{
		// If the Way is the last in the vector or not a polygon return
		if (index == objects.size() - 1 || !objects[index].IsArea())
			return;

		size_t end = index + 25 < objects.size() ? index + 25 : objects.size() - 1;
		// Check the 15 following neighbours of the way at 'index'
		for (size_t i = index + 1; i <= end; i++)
		{
			//std::cout << "Same Type ? " << (objects[i].type == objects[index].type) << std::endl;
			// If both Ways have the same type and are areas try to merge them
			if (objects[i].id != -1 && objects[i].id != -3 && objects[i].type == objects[index].type && objects[i].IsArea())
			{
				vector<size_t> points = vector<size_t>();
				if (MergeAreas(lod, objects[index], objects[i], points))
				{
					// Save new points for the current object
					objects[index].refs = points;
					// Remove duplicates in case polygons shared points
					objects[index].RemoveDuplicates(&m_nodes);
					// Invalidate way used for merging
					objects[i].id = -3;

					//logger.Log(LogLvl::error, "MERGED STUFF");

					if (objects[index].refs.empty() || (objects[index].refs.size() < 4 && types::IsAreaType(objects[index].type)))
						logger.Log(LogLvl::error, "Empty or unclosed Way after merging!");

					bool identical = true;
					for (size_t j = 1; j < objects[index].Size(); j++)
					{
						identical = objects[index].refs[j - 1] == objects[index].refs[j] && identical;
					}

					if (identical)
						logger.Log(LogLvl::error, "All Way points are identical after merging!");

					return;
				}
			}
		}
	}

	void Converter::SortWays(short lod, vector<Way> &objects)
	{
		size_t tile_index = 0;

		logger.Log(LogLvl::info, "Sorting Ways for Tile " + std::to_string(lod));

		if (lod == C_MAX_LOD)
		{
			for (size_t t = 0; t < m_singles.size(); t++)
			{
				tile_index = FindTile(m_singles[t].index, node);
				// If not all tiles are in the tile vector and the tile of interest
				// could not be found an overflow occured and the next steps are skipped
				if (GetOverflow())
					continue;

				if (!m_singles[t].IsInsideTile(m_tiles[tile_index], m_nodes))
					logger.Log(LogLvl::error, 1, "Computed tile does not match single object data");

				m_tiles[tile_index].solo_refs.push_back(t);
			}
		}

		// Reset tile index values
		tile_index = 0;
		// Inspect every way, find its corresponding tile and generalize
		for (size_t i = 0; i < objects.size(); i++)
		{
			if (objects[i].refs.empty() && objects[i].id != -1 && objects[i].id != -3)
			{
				logger.Log(LogLvl::error, "Incorrectly marked empty object in way vector");
			}
			else if (objects[i].id != -1 && objects[i].id != -3)
			{
				if (IsLoDType(lod, objects[i].type))
				{
					// If object is an Area and below a certain Area size try to merge it
					// with neighbouring objects of the same type
					double area_threshold = GetLoDAreaSize(lod);
					if (lod != C_MAX_LOD && !objects[i].IsHouse() && objects[i].IsArea() && objects[i].Area(m_nodes) < area_threshold)
						GeneralizeWays(objects, i, lod);
					// Only include this object if it is big enough measued by Area size
					if (lod == C_MAX_LOD || !objects[i].IsArea() || objects[i].IsHouse() ||(objects[i].IsArea() && objects[i].Area(m_nodes) >= area_threshold))
					{
						if (lod != C_MAX_LOD && objects[i].refs.size() > 4)
						{
							bool restore = false;
							if (objects[i].IsArea() || objects[i].IsCircularWay())
							{
								objects[i].refs.pop_back();
								restore = true;
							}

							if (m_line)
								objects[i].refs = DouglasPeucker(objects[i].refs, GetLoDEpsilon(lod));
							else
								objects[i].refs = VisvalingamWhyatt(objects[i].refs, GetLoDPercentage(lod, objects[i].refs.size()));

							if (restore)
								objects[i].refs.push_back(objects[i].refs[0]);
						}

						tile_index = FindTile(i, way);
						// If not all tiles are in the tile vector and the tile of interest
						// could not be found an overflow occured and the next steps are skipped
						if (GetOverflow())
							continue;

						if (!objects[i].IsInsideTile(m_tiles[tile_index], m_nodes, m_sort))
						{
							logger.Log(LogLvl::warning, 1, "Computed tile does not match way data");

							ofstream of("tile_error.txt", ios_base::app);
							of.precision(8);

							of << "object:\n";
							for (size_t j = 0; j < objects[i].Size(); j++)
							{
								of << "\t" << m_nodes[objects[i].refs[j]].lat << ", " << m_nodes[objects[i].refs[j]].lon << std::endl;
							}

							of << "tile:\n";
							of << "\t" << m_tiles[tile_index].min_lat << ", " << m_tiles[tile_index].max_lat << std::endl;
							of << "\t" << m_tiles[tile_index].min_lon << ", " << m_tiles[tile_index].max_lon << std::endl;

							of.close();
						}

						m_tiles[tile_index].way_refs.push_back(i);
					}
					else if (lod != C_MAX_LOD && objects[i].IsArea() && !objects[i].IsHouse())
					{
						//std::cout << "Objekt = " << objects[i].Area(m_nodes) << ", Grenze = " << area_threshold << std::endl;
					}
				}
			}
		}
	}

	// Relation Generalization
	void Converter::GeneralizeRelations(std::vector<types::Relation> &objects, size_t index, short lod)
	{
		// The new vector of references for this polygon
		vector<size_t> new_refs = vector<size_t>();

		// Check for all members of the relation whether they can be generalized
		for (size_t i = 0; i < objects[index].refs.size(); i++)
		{
			// If we found a way that is an area on its own try to find a merging partner
			if (objects[index].member_types[i] == way && m_ways[objects[index].refs[i]].IsArea())
			{
				size_t end = i + 5 < objects[index].Size() ? i + 5 : objects[index].Size() - 1;
				for (size_t bound = i + 1; bound <= end; bound++)
				{
					// If we found a merging partner try to merge the Wwys
					if (objects[index].member_types[bound] == way && m_ways[objects[index].refs[bound]].IsArea())
					{
						Way merged = Way();
						merged.id = -2;
						merged.type = m_ways[objects[index].refs[i]].type;

						// If merging was successfull save the new polygon to the way vector
						// and store its index in the new reference vector for this relation
						if (MergeAreas(lod, m_ways[objects[index].refs[i]], m_ways[objects[index].refs[bound]], merged.refs))
						{
							m_ways.push_back(merged);
							new_refs.push_back(m_ways.size() - 1);
						}
					}
				}
			}
			// Otherwise just keep the member by pushing it into the new references
			else
			{
				new_refs.push_back(objects[index].refs[i]);
			}
		}
		objects[index].refs = new_refs;
	}

	void Converter::SortRelations(short lod, vector<Relation> &objects)
	{
		size_t tile_index = 0;

		logger.Log(LogLvl::info, "Sorting Relations for Tile " + std::to_string(lod));

		for (size_t i = 0; i < objects.size(); i++)
		{
			if (objects[i].refs.empty() && objects[i].id != -1 && objects[i].id != -3)
			{
				logger.Log(LogLvl::warning, "Incorrectly marked empty object in relation vector");
			}
			else if (objects[i].id != -1 && objects[i].id != -3 && IsLoDType(lod, objects[i].type))
			{
				double area_threshold = GetLoDAreaSize(lod);
				if (lod != C_MAX_LOD && objects[i].IsArea() && objects[i].Area(m_nodes, m_ways, m_relations) < area_threshold)
					GeneralizeRelations(objects, i, lod);
				if (lod == C_MAX_LOD || !objects[i].IsArea() || (objects[i].IsArea() && objects[i].Area(m_nodes, m_ways, m_relations) >= area_threshold))
				{
					if (lod != C_MAX_LOD)
					{
						for (size_t o = 0; o < objects[i].refs.size(); o++)
						{
							// We only need to simplify ways because nodes can't be simplified
							// and member relations will automatically generalized in this function
							if (objects[i].member_types[o] == way)
							{
								size_t save = 0;
								bool restore = false;
								if (m_ways[objects[i].refs[o]].IsArea() || m_ways[objects[i].refs[o]].IsCircularWay())
								{
									m_ways[objects[i].refs[o]].refs.pop_back();
									restore = true;
								}

								if (m_line)
									m_ways[objects[i].refs[o]].refs = DouglasPeucker(m_ways[objects[i].refs[o]].refs, GetLoDEpsilon(lod));
								else
									m_ways[objects[i].refs[o]].refs = VisvalingamWhyatt(m_ways[objects[i].refs[o]].refs, GetLoDPercentage(lod, m_ways[objects[i].refs[o]].refs.size()));

								if (restore)
									m_ways[objects[i].refs[o]].refs.push_back(m_ways[objects[i].refs[o]].refs[0]);
							}
						}
					}

					tile_index = FindTile(i, relation);
					// If not all tiles are in the tile vector and the tile of interest
					// could not be found an overflow occured and the next steps are skipped
					if (GetOverflow())
						continue;

					if (!objects[i].IsInsideTile(m_tiles[tile_index], m_nodes, m_ways, m_relations, m_sort))
						logger.Log(LogLvl::error, 1, "Computed tile does not match relation data");

					m_tiles[tile_index].relation_refs.push_back(i);
				}
			}
		}
	}

	void Converter::SubdivideRelation(types::Tile &lod_tile, Relation &rel)
	{
		Relation other = Relation();
		other.type = rel.type;
		other.id = -2;

		vector<size_t> remaining = vector<size_t>();

		for (size_t i = 0; i < rel.refs.size(); i++)
		{
			if (rel.member_types[i] == way)
			{
				if (!m_ways[rel.refs[i]].IsInsideTile(lod_tile, m_nodes, m_sort))
				{
					other.refs.push_back(rel.refs[i]);
					other.member_types.push_back(way);
					other.roles.push_back(rel.roles[i]);
				}
				else
				{
					size_t count = 0;
					if (m_ways[rel.refs[i]].IsArea())
						count = SubdivideArea(lod_tile, m_ways[rel.refs[i]]);
					else
						count = SubdivideLine(lod_tile, m_ways[rel.refs[i]]);

					for (size_t j = count; j > 0; j--)
					{
						other.refs.push_back(m_ways.size() - j);
						other.member_types.push_back(way);
						other.roles.push_back(rel.roles[i]);
					}
					remaining.push_back(i);
				}
			}
			else if (rel.member_types[i] == relation)
			{
				if (!m_relations[rel.refs[i]].IsInsideTile(lod_tile, m_nodes, m_ways, m_relations, m_sort))
				{
					other.refs.push_back(rel.refs[i]);
					other.member_types.push_back(relation);
					other.roles.push_back(rel.roles[i]);
				}
				else
				{
					SubdivideRelation(lod_tile, m_relations[rel.refs[i]]);
					remaining.push_back(i);
				}

				SubdivideRelation(lod_tile, m_relations[rel.refs[i]]);
			}
			else
			{
				if (!m_nodes[rel.refs[i]].IsInsideTile(lod_tile))
					other.refs.push_back(rel.refs[i]);
				else
					remaining.push_back(i);
			}
		}

		rel.refs.assign(remaining.begin(), remaining.end());
		m_relations.push_back(other);
	}

	// Leftover Way Generalization
	void Converter::SortLeftoverWays(short lod)
	{
		// TODO implement this
	}

	void Converter::UpdateLeftoverWays()
	{
		vector<WayX> update = vector<WayX>(m_ways_left.size() / 2);
		for (size_t i = 0; i < m_ways_left.size(); i++)
		{
			if (!m_ways_left[i].IsComplete())
				update.push_back(m_ways_left.at(i));
		}
		m_ways_left.clear();
		m_ways_left = update;
	}

	// Leftover Relation Generalization
	void Converter::SortLeftoverRelations(short lod)
	{
		// TODO implement this
	}

	void Converter::UpdateLeftoverRelations()
	{
		vector<RelationX> update = vector<RelationX>(m_rels_left.size() / 2);
		for (size_t i = 0; i < m_ways_left.size(); i++)
		{
			if (!m_rels_left[i].IsComplete())
				update.push_back(m_rels_left.at(i));
		}
		m_rels_left.clear();
		m_rels_left = update;
	}
}
