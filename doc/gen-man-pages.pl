#!/usr/bin/env perl
#
# Copyright (c) 2014 Jerry Lundström <lundstrom.jerry@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

use common::sense;
use POSIX;
use XML::LibXML;
use utf8;

foreach my $arg (@ARGV) {
    my $dom = XML::LibXML->load_xml(location => $arg) || die;
    process_dom($dom) || die;
}

sub trim {
    my ($str) = @_;
    $str =~ s/^[ \r\n]+//mgo;
    $str =~ s/[ \r\n]+$//mgo;
    $str =~ s/ +/ /mgo;
    return $str;
}

sub only_detail_text {
    my ($detail) = @_;
    my $detail_text = '';
    
    foreach my $para ($detail->findnodes('para')) {
        my $text = '';
        foreach my $child ($para->childNodes) {
            if ($child->nodeName eq '#text') {
                $text .= $child->textContent;
            }
            elsif ($child->nodeName eq 'ref') {
                $text .= $child->textContent;
            }
        }
        
        $text = trim($text);
        if ($text) {
            if ($detail_text) {
                $detail_text .= '
';
            }
            $detail_text .= $text.'
';
        }
    }
    
    return $detail_text;
}

sub header {
    my ($name, $title) = @_;
    
    print OUTPUT '.\\" Copyright (c) 2014 Jerry Lundström <lundstrom.jerry@gmail.com>
.\\" All rights reserved.
.\\"
.\\" Redistribution and use in source and binary forms, with or without
.\\" modification, are permitted provided that the following conditions
.\\" are met:
.\\" 1. Redistributions of source code must retain the above copyright
.\\"    notice, this list of conditions and the following disclaimer.
.\\" 2. Redistributions in binary form must reproduce the above copyright
.\\"    notice, this list of conditions and the following disclaimer in the
.\\"    documentation and/or other materials provided with the distribution.
.\\"
.\\" THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS\'\' AND ANY EXPRESS OR
.\\" IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
.\\" WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
.\\" ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
.\\" DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
.\\" DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
.\\" GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
.\\" INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
.\\" IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
.\\" OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
.\\" IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
.TH ', uc($name), ' 3 ', strftime('%Y-%m-%d', localtime) ,' "dbo" "dbo API Manual"
.SH NAME
', $name ,' \\- ', $title ,'
';
}

sub footer {
    print OUTPUT '.SH AUTHOR
Jerry Lundström <lundstrom.jerry@gmail.com>
.SH REPORTING BUGS
Please report bugs to https://github.com/jelu/dbo/issues .
.SH COPYRIGHT
Copyright (c) 2014 Jerry Lundström <lundstrom.jerry@gmail.com>. Licensed under the BSD License.
';
}

sub synoptsis {
    my ($sections) = @_;

    print OUTPUT '.SH SYNOPSIS
.nf
';
    
    my %include;
    foreach my $section (@$sections) {
        foreach my $member (@{$section->{members}}) {
            my ($location) = $member->findnodes('location');
            next unless ($location);
            
            my ($file) = $location->attributes->getNamedItem('file');
            next unless ($file);
            
            my $filename = $file->value;
            $filename =~ s/.*\///o;
            next unless ($filename);
            
            $include{$filename} = 1;
        }
    }
    foreach my $include (keys %include) {
        print OUTPUT '.B #include <', $include ,'>
';
    }
    if (%include) {
        print OUTPUT '
';
    }

    my @typedefs;
    foreach my $section (@$sections) {
        foreach my $member (@{$section->{members}}) {
            my ($kind) = $member->attributes->getNamedItem('kind');
            next unless ($kind);
            next unless ($kind->value eq 'typedef');
            my ($type) = $member->findnodes('type');
            my ($name) = $member->findnodes('name');
            
            push(@typedefs, {
                type => $type->textContent,
                name => $name->textContent
            });
        }
    }

    foreach my $typedef (@typedefs) {
        print OUTPUT '.BI "typedef ', $typedef->{type}, ' " ', $typedef->{name},' ;
';
    }
    if (scalar @typedefs) {
        print OUTPUT '
';
    }

    my @enums;
    foreach my $section (@$sections) {
        foreach my $member (@{$section->{members}}) {
            my ($kind) = $member->attributes->getNamedItem('kind');
            next unless ($kind);
            next unless ($kind->value eq 'enum');
            my ($name) = $member->findnodes('name');
            
            my @values = ();
            foreach my $enum ($member->findnodes('enumvalue')) {
                my ($ename) = $enum->findnodes('name');
                
                push(@values, $ename->textContent);
            }
            push (@enums, {name => $name->textContent, values => \@values });
        }
    }
    foreach my $enum (@enums) {
        print OUTPUT '.BI "typedef enum " ', $enum->{name},' " {
.RS
';
        my $first = 1;
        foreach my $value (@{$enum->{values}}) {
            unless ($first) {
                print OUTPUT ',
';
            }
            $first = 0;
            print OUTPUT '.B ', $value;
        }
        print OUTPUT '
.RE
};
';
    }
    if (scalar @enums) {
        print OUTPUT '
';
    }

    my @functions;
    foreach my $section (@$sections) {
        foreach my $member (@{$section->{members}}) {
            my ($kind) = $member->attributes->getNamedItem('kind');
            next unless ($kind);
            next unless ($kind->value eq 'function');
            my ($type) = $member->findnodes('type');
            my ($name) = $member->findnodes('name');
            my @params;
            foreach my $param ($member->findnodes('param')) {
                my ($decl) = $param->findnodes('type');
                my ($declname) = $param->findnodes('declname');
                
                push(@params, { decl => $decl->textContent, ( defined $declname ? ( name => $declname->textContent ) : () ) });
            }
            next unless ($type->textContent and $name->textContent and scalar @params);
            
            push (@functions, {
                name => $name->textContent,
                type => $type->textContent,
                params => \@params
            });
        }
    }
#    my $first = 1;
    foreach my $function (@functions) {
#        unless ($first) {
#            print OUTPUT '
#';
#        }
#        $first = 0;
        
        print OUTPUT '.BI "', $function->{type}, ' ', $function->{name}, '(';
        my $first_param = 1;
        foreach my $param (@{$function->{params}}) {
            unless ($first_param) {
                print OUTPUT ' ", ';
            }
            $first_param = 0;
            
            print OUTPUT $param->{decl}, ($param->{name} ? ' " '.$param->{name}.' ' : '');
        }
        print OUTPUT ');
';
    }

    print OUTPUT '.fi
.sp
Compile and link with \fI\-ldbo\fP.
';
}

sub description {
    my ($detail, $sections) = @_;

    my @typedefs;
    foreach my $section (@$sections) {
        foreach my $member (@{$section->{members}}) {
            my ($kind) = $member->attributes->getNamedItem('kind');
            next unless ($kind);
            next unless ($kind->value eq 'typedef');
            my ($type) = $member->findnodes('type');
            my ($name) = $member->findnodes('name');
            my ($brief) = $member->findnodes('briefdescription');
            my ($detail) = $member->findnodes('detaileddescription');
            
            push(@typedefs, {
                type => $type->textContent,
                name => $name->textContent,
                brief => trim($brief->textContent),
                detail => only_detail_text($detail)
            });
        }
    }

    my @enums;
    foreach my $section (@$sections) {
        foreach my $member (@{$section->{members}}) {
            my ($kind) = $member->attributes->getNamedItem('kind');
            next unless ($kind);
            next unless ($kind->value eq 'enum');
            my ($name) = $member->findnodes('name');
            my ($brief) = $member->findnodes('briefdescription');
            my ($detail) = $member->findnodes('detaileddescription');

            my @values = ();
            foreach my $enum ($member->findnodes('enumvalue')) {
                my ($ename) = $enum->findnodes('name');
                my ($ebrief) = $enum->findnodes('briefdescription');
                my ($edetail) = $enum->findnodes('detaileddescription');
                
                push(@values, {
                    name => $ename->textContent,
                    brief => trim($ebrief->textContent),
                    detail => only_detail_text($edetail)
                });
            }
            push(@enums, {
                name => $name->textContent,
                brief => trim($brief->textContent),
                detail => only_detail_text($detail),
                values => \@values
            });
        }
    }

    my @defines;
    foreach my $section (@$sections) {
        foreach my $member (@{$section->{members}}) {
            my ($kind) = $member->attributes->getNamedItem('kind');
            next unless ($kind);
            next unless ($kind->value eq 'define');
            my ($name) = $member->findnodes('name');
            my ($brief) = $member->findnodes('briefdescription');
            my ($detail) = $member->findnodes('detaileddescription');
            
            push(@defines, {
                name => $name->textContent,
                brief => trim($brief->textContent),
                detail => only_detail_text($detail)
            });
        }
    }

    print OUTPUT '.SH DESCRIPTION
', $detail;

    foreach my $typedef (@typedefs) {
        print OUTPUT '
.B ', $typedef->{name},'
.RS
', ($typedef->{detail} ? $typedef->{detail} : $typedef->{brief}), '
.RE
';
    }

    foreach my $enum (@enums) {
        print OUTPUT '
.B ', $enum->{name},'
.RS
', ($enum->{detail} ? $enum->{detail} : $enum->{brief}), '
';
        foreach my $value (@{$enum->{values}}) {
            print OUTPUT '.TP
.B ', $value->{name},'
', ($value->{detail} ? $value->{detail} : $value->{brief}), '
';
        }
        print OUTPUT '.RE
';
    }

    foreach my $define (@defines) {
        print OUTPUT '.TP
.B ', $define->{name}, '
', ($define->{detail} ? $define->{detail} : $define->{brief}), '
';
    }
}

sub see_also {
    my ($cdetail, $sections) = @_;

    my @functions;
    foreach my $section (@$sections) {
        foreach my $member (@{$section->{members}}) {
            my ($kind) = $member->attributes->getNamedItem('kind');
            next unless ($kind);
            next unless ($kind->value eq 'function');
            my ($name) = $member->findnodes('name');
            push(@functions, $name->textContent);
        }
    }
    
    foreach my $para ($cdetail->findnodes('para')) {
        foreach my $child ($para->childNodes) {
            if ($child->nodeName eq 'simplesect') {
                my ($sectkind) = $child->attributes->getNamedItem('kind');
                
                if ($sectkind->value eq 'see') {
                    push(@functions, $child->textContent);
                }
            }
        }
    }

    return unless (scalar @functions);

    print OUTPUT '.SH SEE ALSO
';
    foreach my $function (@functions) {
        print OUTPUT '.BR ', $function, ' (3)
';
    }
}

sub functions {
    my ($sections) = @_;

    my @functions;
    foreach my $section (@$sections) {
        foreach my $member (@{$section->{members}}) {
            my ($kind) = $member->attributes->getNamedItem('kind');
            next unless ($kind);
            next unless ($kind->value eq 'function');
            my ($name) = $member->findnodes('name');
            next unless ($name);
            
            print 'FUNC: ', $name->textContent, "\n";
            
            my ($mbrief) = $member->findnodes('briefdescription');
            my ($detail) = $member->findnodes('detaileddescription');
            my $brief = $mbrief->textContent;
            $brief =~ s/[\r\n]+/ /mgo;
            $brief =~ s/^ +//go;
            $brief =~ s/[\. ]+$//go;
            $brief =~ s/  +/ /go;
    
            my @sections = ( { sdef => $section->{sdef}, members => [ $member ] } );
            my @see_also;
            
            open(OUTPUT, '>:encoding(UTF-8)', 'man/man3/'.$name->textContent.'.3') || die;
            header($name->textContent, $brief);
            synoptsis(\@sections);
            
            print OUTPUT '.SH DESCRIPTION
';
            my $no_detail = 1;
            foreach my $para ($detail->findnodes('para')) {
                my $detail_text = '';
                
                foreach my $child ($para->childNodes) {
                    if ($child->nodeName eq '#text') {
                        $detail_text .= $child->textContent;
                    }
                    elsif ($child->nodeName eq 'ref') {
                        $detail_text .= $child->textContent;
                    }
                }
                $detail_text = trim($detail_text);
                if ($detail_text) {
                    print OUTPUT $detail_text, '

';
                    $no_detail = 0;
                }
            }
            if ($no_detail) {
                print OUTPUT trim($mbrief->textContent), '

';
            }

            my $return_text = '';
            foreach my $para ($detail->findnodes('para')) {
                foreach my $child ($para->childNodes) {
                    if ($child->nodeName eq 'simplesect') {
                        my ($sectkind) = $child->attributes->getNamedItem('kind');
                        
                        if ($sectkind->value eq 'return') {
                            $return_text .= $child->textContent;
                        }
                    }
                }
            }
            if ($return_text) {
                print OUTPUT '.SH RETURN VALUE
', trim($return_text), '
';
            }

            my @see;
            foreach my $para ($detail->findnodes('para')) {
                foreach my $child ($para->childNodes) {
                    if ($child->nodeName eq 'simplesect') {
                        my ($sectkind) = $child->attributes->getNamedItem('kind');
                        
                        if ($sectkind->value eq 'see') {
                            push(@see, $child->textContent);
                        }
                    }
                }
            }
            if (scalar @see) {
                print OUTPUT '.SH SEE ALSO
';
                foreach my $see (@see) {
                    print OUTPUT '.BR ', $see, ' (3)
';
                }
            }

            footer;
            close(OUTPUT);
        }
    }
}

sub process_dom {
    my $dom = shift || die;
    
    foreach my $cdef ($dom->findnodes('/doxygen/compounddef')) {
        my ($ckind) = $cdef->attributes->getNamedItem('kind');
        if ($ckind->value ne 'group') {
            next;
        }
        my ($cname) = $cdef->findnodes('compoundname');
        die unless($cname->textContent);
        my ($ctitle) = $cdef->findnodes('title');
        my ($cbrief) = $cdef->findnodes('briefdescription');
        my ($cdetail) = $cdef->findnodes('detaileddescription');
        my $brief = $cbrief->textContent;
        $brief =~ s/[\r\n]+/ /mgo;
        $brief =~ s/^ +//go;
        $brief =~ s/[\. ]+$//go;
        $brief =~ s/  +/ /go;
        
        print 'COMPOUND: ', $cname->textContent, "\n",
            'TITLE: ', $ctitle->textContent, "\n";
        
        my @sections = ();
        foreach my $sdef ($cdef->findnodes('sectiondef')) {
            my ($skind) = $sdef->attributes->getNamedItem('kind');
            
            print 'SECTION: ', $skind->value, "\n";
            
            my @members = ();
            foreach my $mdef ($sdef->findnodes('memberdef')) {
                my ($mkind) = $mdef->attributes->getNamedItem('kind');
                my ($mname) = $mdef->findnodes('name');
                
                print '    MEMBER: ', $mname->textContent, ' (', $mkind->value, ")\n";
                
                push(@members, $mdef);
            }
            
            push(@sections, { sdef => $sdef, members => \@members });
        }

        my $detail = only_detail_text($cdetail);
        if (!$detail) {
            $detail = trim($cbrief->textContent);
        }
        
        open(OUTPUT, '>:encoding(UTF-8)', 'man/man3/'.$cname->textContent.'.3') || die;
        header($ctitle->textContent, $brief);
        synoptsis(\@sections);
        description($detail, \@sections);
        see_also($cdetail, \@sections);
        footer;
        close(OUTPUT);
        
        functions(\@sections);
    }
    
    return 1;
}
