#============================================================#
#                                                            #
# $ID$                                                       #
#                                                            #
# NaElement.rb                                               #
#                                                            #
# Operations on ONTAPI and DataFabric Manager elements       #
#                                                            #
# Copyright (c) 2011 NetApp, Inc. All rights reserved.       #
# Specifications subject to change without notice.           #
#                                                            #
#============================================================#

#Class encapsulating Netapp XML request elements.
#An NaElement encapsulates one level of an XML element.
#Elements can be arbitrarily nested.  They have names,
#corresponding to XML tags, attributes (only used for
#results), values (always strings) and possibly children,
#corresponding to nested tagged items.  See NaServer for
#instructions on using NaElements to invoke ONTAPI API calls.
#The following routines are available for constructing and
#accessing the contents of NaElements.
    
class NaElement 
    
  #Global Variables
  DEFAULT_KEY = "#u82fyi8S5\017pPemw"
  MAX_CHUNK_SIZE = 256

  #Construct a new NaElement.  The 'value' parameter is
  #optional for top level elements.
     
  def initialize(name, content = "")
      @name = name
      @content = content
      @children = []
      @attrkeys = []
      @attrvals = []
  end

	
  #Indicates success or failure of API call.
  #Returns either 'passed' or 'failed'.
       
  def results_status()
      r = attr_get("status")
      if(r.eql?("passed"))
	return "passed"
      else
	return "failed"
      end
  end

  #Human-readable string describing a failure.
  #Only present if results_status does not return 'passed'.
   
  def results_reason()
      r = attr_get("status")
      if(r.eql?("passed"))
	return nil
      end
      r = attr_get("reason")
      unless r
	return "No reason given"
      end
      return r.to_s
  end

        
  #Returns an error number, 0 on success.
     
  def results_errno()		
      r = attr_get("status")	
      if(r.eql?("passed"))
	return 0
      end	
      r = attr_get("errno")
      unless r
	r = -1
      end	
      return r
  end
        
	
  #Get a named child of an element, which is also an
  #element.  Elements can be nested arbitrarily, so
  #the element you get with this could also have other
  #children.  The return is either an NaElement named
  #name', or None if none is found.
     		
  def child_get(name)
      @children.each do |elem|
          if(name.eql?(elem.name))
	      return elem
          end
      end
      return nil
  end

	
  #Set the element's value to 'content'.  This is
  #not needed in normal development.
        
  def set_content(content)
      @content = content
  end


  #Add the element's value to 'content'.  This is
  #not needed in normal development.
        
  def add_content(content)
      @content = @content + content
  end

	
  #Returns 1 if the element has any children, 0 otherwise
	
  def has_children()
      if(@children.length > 0)
	return 1
      else
	return 0
      end
  end


  #Add the element 'child' to the children list of
  #the current object, which is also an element.
        
  def child_add(child)
      @children.push(child)
  end


  #Construct an element with name 'name' and contents
  #'value', and add it to the current object, which
  #is also an element.
        
  def child_add_string(name, value)
      elt = NaElement.new(name, value)
      child_add(elt)
  end


  #Gets the child named 'name' from the current object
  #and returns its value.  If no child named 'name' is
  #found, returns None.
        
  def child_get_string(name)
      @children.each do |elem|
	if(name.eql?(elem.name))
	    return elem.content
	end
      end	
      return nil
  end
	
	
  #Gets the child named 'name' from the current object
  #and returns its value as an integer.  If no child
  #named 'name' is found, returns None.
        
  def child_get_int(child)
      temp =  child_get_string(child)
      return temp.to_i
  end


  #Returns the list of children as an array.

  def children_get()       	
      return @children
  end
	

  #Sprintf pretty-prints the element and its children,
  #recursively, in XML-ish format.  This is of use
  #mainly in exploratory and utility programs.  Use
  #child_get_string() to dig values out of a top-level
  #element's children.
  #Parameter 'indent' is optional.
	
  def sprintf(indent = "")       
      s = indent + "<" + @name
      key_count = 0		
      @attrkeys.each do |key|
	s = s + " " + key.to_s + "=\"" + @attrvals[key_count].to_s + "\""
	key_count = key_count + 1
      end		
      s = s + ">"		
      if(@children.size() > 0)
	s = s + "\n"
      end	
      @children.each do |child| 
	unless(child.class.name.eql?("NaElement"))
	abort("Unexpected reference found, expected NaElement not " + child.class.name)
        end
        s = s + child.sprintf(indent + "\t")
      end		
      @content = NaElement::escapeHTML(@content)
      s = s + @content.to_s
      if(@children.size() > 0)
	s = s + indent
      end
      s = s + "</" + @name + ">\n"
      return s
  end
	
	
  #Same as child_add_string, but encrypts 'value'
  #with 'key' before adding the element to the current
  #object.  This is only used at present for certain
  #key exchange operations.  Both client and server
  #must know the value of 'key' and agree to use this
  #routine and its companion, child_get_string_encrypted().
  #The default key will be used if the given key is None.
        
  def child_add_string_encrypted(name, value, key = nil)
      if(not name or not value)
          abort("Invalid input specified for name or value")
      end
      unless(key)
          key = DEFAULT_KEY
      end
      if (key.length != 16)
          abort("Invalid key, key length sholud be 16")
      end
      #encryption of key and others
      encrypted_value = RC4(key, value)
      unpacked_value = encrypted_value.unpack('H*')
      child_add_string(name, unpacked_value[0])
  end

	
  #Get the value of child named 'name', and decrypt
  #it with 'key' before returning it.
  #The default key will be used if the given key is None.
        
  def child_get_string_encrypted(name, key = nil)
      if (key == nil)
          key = DEFAULT_KEY
      end
      if (key.length != 16)
          abort("Invalid key, key length sholud be 16")
      end
      value = child_get_string(name)
      value_array = value.lines.to_a
      plaintext = RC4(key, value_array.pack('H*'))
      return plaintext
  end


  #Encodes string embedded with special chars like &,<,>.
  #This is mainly useful when passing string values embedded
  #with special chars like &,<,> to API.
  #Example :
  #server.invoke("qtree-create","qtree","abc<qt0","volume","vol0")
	
  def toEncodedString()       
      s = "<" + @name.to_s
      key_count = 0		
      @attrkeys.each do |key|
	s = s + " " + key.to_s + "=\"" + @attrvals[key_count].to_s + "\""
	key_count = key_count + 1
      end		
      s = s + ">"	
      @children.each do |child|
	unless(child.class.name.eql?("NaElement"))
	  abort("Unexpected reference found, expected NaElement not " + child.class.name)
        end
        s = s + child.toEncodedString()
      end		
      cont = @content.to_s
      cont = NaElement::escapeHTML(cont) 
      s = s + cont
      s = s + "</" + @name.to_s + ">"
      return s
  end

  # This is a private function, not to be called externally.
  # This method converts reserved HTML characters to corresponding entity names.
  def NaElement::escapeHTML(cont)
	cont = cont.gsub('&','&amp;')
	cont = cont.gsub('<','&lt;')
	cont = cont.gsub('>','&gt;')
	cont = cont.gsub("'",'&apos;')
	cont = cont.gsub('"','&quot;')
	
	# The existence of '&' (ampersand) sign in entity names implies that multiple calls
	# to this function will result in non-idempotent encoding. So, to handle such situation
	# or when the input itself contains entity names, we reconvert such recurrences to
	# appropriate characters.
	cont = cont.gsub('&amp;amp;','&amp;')
	cont = cont.gsub('&amp;lt;','&lt;')
	cont = cont.gsub('&amp;gt;','&gt;')
	cont = cont.gsub('&amp;apos;','&apos;')
	cont = cont.gsub('&amp;quot;','&quot;')
	
	return cont
  end
  		
    
  #This is a private function, not to be called from outside NaElement.
	
  def RC4(key, value)		
	box = prepare_key(key)
	x,y = 0,0
	plaintext = value
	num = plaintext.length.to_f/MAX_CHUNK_SIZE
        integer = num.to_i
        if(integer == num)
            num_pieces = integer
        else 
            num_pieces = integer + 1
	end
	plaintext_array = []
	num_pieces = num_pieces - 1
        for piece in (0..num_pieces) do		
            plaintext_array = plaintext[piece * MAX_CHUNK_SIZE, (piece * MAX_CHUNK_SIZE) + MAX_CHUNK_SIZE].unpack("C*")
	    index  = 0
            for i in plaintext_array do
		x = x + 1
                if (x > 255 )
                    x = 0
		end				
                y = y + box[x]
                if(y > 255)
                    y = y - 256
		end
                box[x], box[y] = box[y], box[x]
                plaintext_array[index] = (i ^ (box[(box[x] + box[y]) % 256]))
		index = index + 1
            end			
            plaintext[piece * MAX_CHUNK_SIZE, (piece * MAX_CHUNK_SIZE) + MAX_CHUNK_SIZE] = plaintext_array.pack("C*")
	end		
        return plaintext
  end
	
	
  def prepare_key(key)
	k = key.unpack('C*')
	box = (0..255).to_a
	y = 0	
	index = 0
	for x in 0..255 do
	    index = x%(k.length)
	    y = (k[index] + box[x].to_i + y ) % 256
	    box[x], box[y] = box[y], box[x]
	end		
	return box
  end		
		
	
  #This is a private function, not to be called from outside NaElement.
    
  def attr_set(key, value)        
        @attrkeys.push(key)
	@attrvals.push(value)
  end


  #This is a private function, not to be called from outside NaElement.
	
  def attr_get(key)
       key_count = 0
       @attrkeys.each do |attrkey|			
	 if(attrkey == key)
  	   return @attrvals[key_count]
	 end
	 key_count = key_count + 1
       end
       return nil
  end
	
  def name
	return  @name
  end
	
  def content
	return  @content
  end
end













