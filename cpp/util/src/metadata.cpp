#include "metadata.h"

Metadata::Metadata()
{
	// Write the begin document flag to the Yaml object.
	emitter_ << YAML::BeginDoc;
}

ManagedPath Metadata::GetYamlMetadataPath(const ManagedPath& output_dir,
	const std::string& base_file_name)
{
	ManagedPath file_name_component;
	if (base_file_name != "")
		file_name_component = ManagedPath(std::string(base_file_name + ".yaml"));
	else
		file_name_component = ManagedPath(std::string("_metadata.yaml"));
	return output_dir / file_name_component;
}

std::string Metadata::GetMetadataString()
{
	// Write the end document Yaml flag.
	emitter_ << YAML::EndDoc;

	std::string metadata(emitter_.c_str());
	return metadata;
}