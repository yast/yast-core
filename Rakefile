require "yast/rake"

Yast::Tasks.configuration do |conf|
  conf.skip_license_check << /testsuite/ #skip all testsuites
  conf.skip_license_check << /scr\/doc\/scr-arch.sda/ #skip binary file
  conf.skip_license_check << /.*/ #skip all temporary, as there is too much failures
  conf.obs_project = "YaST:Head:installer"
  conf.obs_target = "openSUSE_13.1"
end

