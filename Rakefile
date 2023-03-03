require "yast/rake"

Yast::Tasks.submit_to :sle15sp5

Yast::Tasks.configuration do |conf|
  conf.skip_license_check << /testsuite/ #skip all testsuites
  conf.skip_license_check << /scr\/doc\/scr-arch.sda/ #skip binary file
  conf.skip_license_check << /.*/ #skip all temporary, as there is too much failures
end

