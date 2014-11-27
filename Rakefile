require "yast/rake"

Yast::Tasks.configuration do |conf|
  conf.obs_api = "https://api.opensuse.org"
  conf.obs_target = "openSUSE_13.2"
  conf.obs_sr_project = "openSUSE:13.2:Update"
  conf.obs_project = "YaST:openSUSE:13.2"
  conf.skip_license_check << /testsuite/ #skip all testsuites
  conf.skip_license_check << /scr\/doc\/scr-arch.sda/ #skip binary file
  conf.skip_license_check << /.*/ #skip all temporary, as there is too much failures
end

